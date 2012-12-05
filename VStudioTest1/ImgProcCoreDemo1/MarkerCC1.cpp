#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <stdlib.h>
#include <math.h>
#include "TwoColorLocator.h"
#include "MarkerCC1.h"

#include "TimeMeasurementCodeDefines.h"

#define COLORCODE_INITIAL 254	// Used to indicate no valid value, even no "unrecognized color"
#define COLORCODE_NONE 255
#define COLORCODE_BLU 0
#define COLORCODE_RED 1
#define COLORCODE_GRN 2

#define MAXINVALIDCOLORNUM 10

using namespace cv;
using namespace std;

// Meg kell hivni, mielott az alabbi fuggvenyeket hasznaljuk... (2 bites kodok Hue ertekhez rendelese)
void MarkerCC1::initHue2CodeLut()
{
	for(int i=0; i<256; i++)
	{
		if (100 <= i && i <= 180)
		{
			hue2codeLUT[i] = COLORCODE_BLU;
		}
		else if (1 <= i && i <= 15)
		{
			hue2codeLUT[i] = COLORCODE_RED;
		}
		else if (80 <= i && i <= 95)
		{
			hue2codeLUT[i] = COLORCODE_GRN;
		}
		else
		{
			hue2codeLUT[i] = COLORCODE_NONE;
		}
	}
}

void MarkerCC1::configTwoColorFilter(TwoColorFilter *filter)
{
	// Sets parameters to conform marker colors
	// Blue filter (109-111) 
	filter->hue2Filter->lowThreshold = 100;
	filter->hue2Filter->highThreshold = 180;
	// RED filter (3-5)
	filter->hue1Filter->lowThreshold = 1;
	filter->hue1Filter->highThreshold = 10;
	// Sat filter (154-)
	filter->satFilter->threshold = 100;
	// Val filter (154-)
	filter->valFilter->threshold = 75;
}


// Entry of marker identification
// It needs a hue image
// Egy potencialis marker helyet dolgoz fel.
// calls: readCodeAlongLine, validateAndConsolidateMarkerCode
void MarkerCC1::readCode(Mat &hueSmaskImg, Mat &valImg, CvRect &rect, Mat *verboseImage)
{
	CV_Assert(hueSmaskImg.depth() == CV_8U);	// Assuming every pixel to have 8 bits...
	// TODO: draw on debugImage if not NULL...

	// Init marker
	center.x = rect.x + rect.width/2;
	center.y = rect.y + rect.height/2;
//	for(int i=0; i<8; i++) codeArray[i]=0;
	isValid = false;	// Not valid, as not set...

	scanDistance = rect.width>rect.height ? 2*rect.width : 2*rect.height;

	// SCAN
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerScanlines);
	for (int dir=0; dir<8; dir++)
	{
		if (!findBordersAlongLine(hueSmaskImg,dir, verboseImage))
		{
			return;	// Marker direction rejected...
		}
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::MarkerScanlines);
	// if we get here, all directions were successfully processed and the borders detected.

	// Fit ellipses to borders
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerFitEllipses);
	fitBorderEllipses(verboseImage);
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerFitEllipses);

	// Read code along ellipses
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerScanEllipses);
	scanEllipses(valImg,hueSmaskImg,verboseImage);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::MarkerScanEllipses);

	// TODO: Validate marker code
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ConsolidateValidate);
	validateAndConsolidateMarkerCode();
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ConsolidateValidate);

	char tmpCodeString[255];
	sprintf(tmpCodeString,"%d-%d",majorMarkerID,minorMarkerID);
	string debugtxt(tmpCodeString);

	if (verboseImage != NULL && !disableVerboseEllipses)
	{
		if (isValid)
		{
			putText(*verboseImage,debugtxt,center,FONT_HERSHEY_PLAIN,1,Scalar(0,255,0),2);
		}
		else
		{
			putText(*verboseImage,debugtxt,center,FONT_HERSHEY_PLAIN,1,Scalar(0,0,255),2);
		}
	}
}


// Fit ellipses on borders which where found by findBordersAlongLine()
// and stored in RedInnerBorders[8] and RedOuterBorders[8].
void MarkerCC1::fitBorderEllipses(Mat *verboseImage)
{
	// Assuming that the borders in both directions were found.
	vector<Point> innerBorderPoints;
	innerBorderPoints.assign(this->RedInnerBorders,this->RedInnerBorders+8);
	innerEllipse = fitEllipse(innerBorderPoints);

	vector<Point> outerBorderPoints;
	outerBorderPoints.assign(this->RedOuterBorders,this->RedOuterBorders+8);
	outerEllipse = fitEllipse(outerBorderPoints);

	if (verboseImage != NULL)
	{
		ellipse(*verboseImage,innerEllipse,Scalar(200,200,200));
		ellipse(*verboseImage,outerEllipse,Scalar(255,255,255));
	}

}

// Uses innerEllipse and outerEllipse and dirWithGreen to
//	scan the binary code along the external parts of the marker
void MarkerCC1::scanEllipses(Mat &valSrc, Mat& hueSmaskSrc, Mat *verboseImage)
{
	// The center of the marker is the mean of the ellipse centers
	Point markerCenter = Point(
		(innerEllipse.center.x + outerEllipse.center.x) / 2.0,
		(innerEllipse.center.y + outerEllipse.center.y) / 2.0);

	// TODO: now using the angle of the inner ellipse only. Should be more
	//	rebust by using the mean direction (have to handle the wrap-around)!
	Size2f baseSize(
		(innerEllipse.size.width + outerEllipse.size.width/2) / 2,
		(innerEllipse.size.height + outerEllipse.size.height/2) / 2);
	RotatedRect baseEllipse(markerCenter,baseSize,outerEllipse.angle);

	if (verboseImage!=NULL)
	{
		Point2f vertices[4];
		baseEllipse.points(vertices);
		for (int i = 0;i < 4;i++)
			line(*verboseImage,vertices[i],vertices[(i+1)%4],Scalar(0,255,0));

		// Show ellipse parameters
		//Point lineEnd = Point( markerCenter.x + 2*horizontalRadius*sin(ellipseAngle/CV_PI*180), markerCenter.y - 2*horizontalRadius*cos(ellipseAngle/CV_PI*180) );
		Point lineEnd = Point(
			markerCenter.x + 50*sin(baseEllipse.angle/180.0*CV_PI),
			markerCenter.y - 50*cos(baseEllipse.angle/180.0*CV_PI) );
		line(*verboseImage,markerCenter,lineEnd,Scalar(255,255,255));
	}

	int bitIdx = 0;
	// Inner scanning ellipse
	for(float directionAngle=0.0; directionAngle<359.0; directionAngle += 45.0)
	{
		Point location = getEllipsePointInDirection(baseEllipse,directionAngle,2.0);
		rawMarkerIDBitVal[bitIdx]=valSrc.at<uchar>(location);
		rawMarkerIDBitHueSmask[bitIdx]=hueSmaskSrc.at<uchar>(location);
		bitIdx++;
		if (verboseImage!=NULL)
		{
			circle(*verboseImage,location,3,Scalar(255,255,255));
		}
	}
	// Outer scanning ellpise
	for(float directionAngle=0.0; directionAngle<359.0; directionAngle += 22.5)
	{
		Point location = getEllipsePointInDirection(baseEllipse,directionAngle,3.0);
		rawMarkerIDBitVal[bitIdx]=valSrc.at<uchar>(location);
		rawMarkerIDBitHueSmask[bitIdx]=hueSmaskSrc.at<uchar>(location);
		bitIdx++;
		if (verboseImage!=NULL)
		{
			circle(*verboseImage,location,3,Scalar(255,255,255));
		}
	}
}

Point MarkerCC1::getEllipsePointInDirection(RotatedRect baseEllipse,float directionAngle,float distanceMultiplier)
{
	// TODO: this takes lots of time now...

	// Calculating distance from origin based on an unrotated ellipse
	float x = baseEllipse.size.width/2 * sin( directionAngle /180.0*CV_PI );
	float y = baseEllipse.size.height/2 * cos( directionAngle /180.0*CV_PI );
	float distance = sqrt( x*x + y*y ) * distanceMultiplier;

	// Angle is in degrees, starting from north and increasing clockwise.
	float finalX = baseEllipse.center.x + distance * sin( (baseEllipse.angle + directionAngle) /180.0*CV_PI);
	float finalY = baseEllipse.center.y - distance * cos( (baseEllipse.angle + directionAngle) /180.0*CV_PI);// * hwRatio;
	return Point((int)finalX,(int)finalY);
/*	x += baseEllipse.center.x;
	y += baseEllipse.center.y;
	return Point(x,y);*/
}


// Egy iranybe vegigolvassa a szineket (meg jo esellyel kicsit tovabb is.)
// Kitolti a marker.codeArray -t.
// Performs center color validation!
// Return: false for immediate reject, True: may be valid marker...
bool MarkerCC1::findBordersAlongLine(Mat &hueSmaskImg, int dir, Mat *verboseImage)
{
	// Szinenkent 2 bitet kodolunk
	// Max. MaxValuePerLine db eltero es felismert szint olvasunk be egymas utan a vonal menten.
	const int MaxValuePerLine = 4;	// max. 2x4=8 bitet hasznalunk el minden iranyban (meg nem nezzuk, meddig hasznos!)

	// TODO: vajon van annyi esze, hogy nem megy ki a kepbol?!
	CvPoint endpoint = getEndPoint(center.x, center.y, scanDistance, dir);

	
	// Meanwhile, also find inner and outer borders of RED (value 1) area
	// Fills RedInnerBorders[dir] and RedOuterBorders[]
	int currentAreaColorCode = COLORCODE_INITIAL;
	bool isDirectionValid = false;	// True if BLUE and RED area has been found. False otherwise.

	// Internal valiables to track changes and last locations
	uchar colorValue = -1;		// Color code for current pixel
	uchar prevColorValue = -1;	// Used to suppress 1 pixel errors
	Point prevPoint;			// Location of previous point
	int invalidPixelColorCounter = 0;		// Couting the number of pixels with invalid color. Beyond MAXINVALIDCOLORNUM, we do not search for green anymore...

//	bool isGreenInThisDirection = false;	// True if green color was found directly beyond the red area

	// Scan along the given line and store the color codes (using hue2codeLUT)
	LineIterator lineIt(hueSmaskImg,center,endpoint);
	int length = lineIt.count;
	for(int i=0; i<length; i++,lineIt++)
	{
		uchar hue = *lineIt.ptr;
		colorValue = hue2codeLUT[hue];	// -1 means invalid color, valids are 0-3

		if (colorValue == COLORCODE_NONE)
		{
			invalidPixelColorCounter++;
			if (invalidPixelColorCounter>MAXINVALIDCOLORNUM)
			{
				// No more valuable pixels. (May already be OK!)
				break;
			}
		}

		// Analyse only if successive value is the same
		if (colorValue != prevColorValue)
		{
			// Changing color, storing previous color and going on
			prevPoint = lineIt.pos();
			prevColorValue = colorValue;
			continue;
		}
		// (Remark: cannot stop here with COLORCODE_NONE, as that marks the end of the red area...)

		// Now successive pixels have same color code
		// Now comes a Finite State Maching
		// State variable: currentAreaColorCode
		if (colorValue != currentAreaColorCode)	// If something may  change...
		{
			switch(currentAreaColorCode)
			{
			case COLORCODE_INITIAL:
				// In the middle, we have to find blue color. Otherwise, the marker is rejected.
				if (colorValue != COLORCODE_BLU)
				{
					//cout << "Reject ->nonBLU, dir="<<dir<<endl;
					return false;	// Immediate reject
				}
				currentAreaColorCode = COLORCODE_BLU;
				break;
			case COLORCODE_BLU:
				// This is the initial area code
				switch (colorValue)
				{
				case COLORCODE_RED:
					// If we found RED, we jump into the red area and store the entry point
					RedInnerBorders[dir] = prevPoint;	// The previous point was already red!
					currentAreaColorCode=COLORCODE_RED;
					break;
				case COLORCODE_GRN:
					// Green cannot come here. If it does, marker is rejected.
					//cout << "Reject BLU->GRN, dir="<<dir<<endl;
					return false;	// Immediate reject
				}
				break;
			case COLORCODE_RED:
				// If we achieved the red area, we search for its end. Green or none color code may come after this.
				switch(colorValue)
				{
				case COLORCODE_GRN:
					RedOuterBorders[dir] = prevPoint;	// The previous point was already green!
					isDirectionValid = true;	// Marker direction is now valid.
					currentAreaColorCode=COLORCODE_GRN;
					break;
				case COLORCODE_BLU:
					// Blue cannot come here. Marker is rejected
					//cout << "Reject RED->BLU, dir="<<dir<<endl;
					return false;	// Immediate reject
					break;
				case COLORCODE_NONE:
					// No recognized color means the end of the red area.
					RedOuterBorders[dir] = prevPoint;	// The previous point was already outside the RED area.
					isDirectionValid = true;	// Marker direction is now valid.
					currentAreaColorCode = COLORCODE_NONE;
					break;
				}
				break;
			case COLORCODE_GRN:
//				isGreenInThisDirection = true;
//				dirWithGreen = dir;	// Handles multiple such directions, last processed one overwrites previous ones.
				//cout << "DirGrn="<<dir<<endl;
				break;
			}
		}

		// Show small circle with color representing current finite state machine state
		if (verboseImage != NULL  && !disableVerboseScanlines)
		{
			//switch(currentAreaColorCode)
			switch(colorValue)
			{
			case COLORCODE_BLU:
				circle(*verboseImage, lineIt.pos(), 2, Scalar(255,100,100));	// In BGR!
				break;
			case COLORCODE_RED:
				circle(*verboseImage, lineIt.pos(), 2, Scalar(100,100,255));	// In BGR!
				break;
			case COLORCODE_GRN:
				circle(*verboseImage, lineIt.pos(), 2, Scalar(100,255,100));
				break;
			case COLORCODE_NONE:
				circle(*verboseImage, lineIt.pos(), 2, Scalar(100,100,100));
				break;
			case COLORCODE_INITIAL:
				circle(*verboseImage, lineIt.pos(), 2,  Scalar(0,0,0));
				break;
			}
		}
	}

	// Eredmény mentése a marker-be... (a végén még lehetnek hibás bitek!)
	// Meg igazabol a kozeppont szinet is ki lehet dobni az LSB-nel levo 2 bitrol...
	return isDirectionValid;
}

CvPoint MarkerCC1::getEndPoint(int x, int y, int distance, int dir)
{
	CV_Assert(dir>=0 && dir<8);

	const int dirXMul[] = {0,1,1,1,0,-1,-1,-1};
	const int dirYMul[] = {-1,-1,0,1,1,1,0,-1};

	// TODO: vajon van annyi esze, hogy nem megy ki a kepbol?!
	CvPoint endpoint = cvPoint(
		x + dirXMul[dir]*distance,
		y + dirYMul[dir]*distance);

	return endpoint;
}

// After the raw bits are read (oversampled),
//	this method calculates the final marker ID and validates it.
void MarkerCC1::validateAndConsolidateMarkerCode()
{
	// Find minimal and maximal bit value to adapt threshold into the middle...
	int min=255;
	int max=0;
	for (int bitIdx=0; bitIdx<24; bitIdx++)
	{
		if (min>rawMarkerIDBitVal[bitIdx])
			min=rawMarkerIDBitVal[bitIdx];
		if (max<rawMarkerIDBitVal[bitIdx])
			max=rawMarkerIDBitVal[bitIdx];
	}
	int threshold = (min+max)/2;
	//cout << "Value:"<<min<<"-"<<max<<",th="<<threshold;

	// Get binary bits and find green color
	int rawbits[24];
	int greenIdx = -1;	// Index of last green location
	//cout << "rawBits:";
	for (int bitIdx=0; bitIdx<24; bitIdx++)
	{
		rawbits[bitIdx] = rawMarkerIDBitVal[bitIdx]>threshold ? 1 : 0;
		//cout << rawbits[bitIdx];

		int colorCode = hue2codeLUT[rawMarkerIDBitHueSmask[bitIdx]];
		// Search for green in the inner ellipse
		if (colorCode == COLORCODE_GRN)
		{
			greenIdx=bitIdx;
		}
	}
	//cout << ", GRN@" << greenIdx << endl;

	// Collect bits beginning from green location.
	if (greenIdx<8)	// If green is not on the outer ellipse, marker ID cannot be valid.
	{
		isValid = false;
		majorMarkerID=0;
		minorMarkerID=0;
		return;
	}

	int realBitIdxInner[4];
	int realBitIdxOuter[8];
	int finalInnerBits[4];
	int finalOuterBits[8];
	unsigned int innerCode = 0;
	unsigned int outerCode = 0;
	//cout << "GrnIdx:" << greenIdx << ", Outer code: ";
	for(int i=0; i<8; i++)
	{
		realBitIdxOuter[i] = 8 + ((greenIdx-8)+2*i) % 16;
		finalOuterBits[i] = rawbits[realBitIdxOuter[i]];
		//cout << finalOuterBits[i];
		outerCode |= finalOuterBits[i];
		if (i<7)
			outerCode <<= 1;
	}
	//cout << ", inner code: ";
	for(int i=0; i<4; i++)
	{
		realBitIdxInner[i] = (realBitIdxOuter[2*i]-8)/2;
		finalInnerBits[i] = rawbits[realBitIdxInner[i]];
		//cout << finalInnerBits[i];
		innerCode |= finalInnerBits[i];
		if (i<3)
			innerCode <<= 1;
	}
	//cout << ", iCode=" << innerCode << " oCode=" << outerCode << endl;

	// TODO: vegso soron a marker.markerID -t kell beallitani.
	majorMarkerID = innerCode;
	minorMarkerID = outerCode;
	isValid = true;
}

