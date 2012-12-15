#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <stdlib.h>
#include <math.h>
#include "FastColorFilter.h"	// For color codes
#include "TwoColorLocator.h"
#include "MarkerCC1.h"

#include "TimeMeasurementCodeDefines.h"
#include "ConfigManager.h"

#define COLORCODE_INITIAL 254	// Used to indicate no valid value, even no "unrecognized color"
#define MAXINVALIDCOLORNUM 10	// Line scanning stops after so many pixels with invalid color (used by ellipse fitting)

using namespace cv;
using namespace std;
using namespace TwoColorCircleMarker;


// Entry of marker identification
void MarkerCC1::readCode(Mat &srcCC, CvRect &rect)
{
	CV_Assert(srcCC.depth() == CV_8U);	// Assuming every pixel to have 8 bits...

	// Init marker
	center.x = rect.x + rect.width/2;
	center.y = rect.y + rect.height/2;
	isValid = false;	// Not valid, as not set...

	// --- Estimate red circle inner and outer ellipses
	// Maximal distance of line scanning
	scanDistance = rect.width>rect.height ? 2*rect.width : 2*rect.height;

	// Scan lines in 8 directions
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerScanlines);
	for (int dir=0; dir<8; dir++)
	{
		if (!findBordersAlongLine(srcCC,dir))
		{
			return;	// Marker direction rejected...
		}
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::MarkerScanlines);
	// If we get here, all directions were successfully processed and the borders detected.

	// Fit ellipses to borders
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerFitEllipses);
	fitBorderEllipses();
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerFitEllipses);

	// --- Read marker code along elliptical curves
	// Read code along ellipses
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::MarkerScanEllipses);
	scanEllipses(srcCC);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::MarkerScanEllipses);

	// --- Analyse and validate the ellpise reading results
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ConsolidateValidate);
	validateAndConsolidateMarkerCode();
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ConsolidateValidate);

	// --- Verbose
	if (verboseImage != NULL && ConfigManager::Current()->showMarkerCodeOnImage)
	{
		char tmpCodeString[255];
		sprintf(tmpCodeString,"%d-%d",majorMarkerID,minorMarkerID);
		string debugtxt(tmpCodeString);

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

// -------------------------------------- internal functions ----------------------------------

// Fit ellipses on borders which where found by findBordersAlongLine()
// and stored in RedInnerBorders[8] and RedOuterBorders[8].
void MarkerCC1::fitBorderEllipses()
{
	// Assuming that the borders in both directions were found.
	vector<Point> innerBorderPoints;
	innerBorderPoints.assign(this->RedInnerBorders,this->RedInnerBorders+8);
	innerEllipse = fitEllipse(innerBorderPoints);

	vector<Point> outerBorderPoints;
	outerBorderPoints.assign(this->RedOuterBorders,this->RedOuterBorders+8);
	outerEllipse = fitEllipse(outerBorderPoints);

	if (verboseImage != NULL && ConfigManager::Current()->verboseEllipseFitting)
	{
		ellipse(*verboseImage,innerEllipse,Scalar(200,200,200));
		ellipse(*verboseImage,outerEllipse,Scalar(255,255,255));
	}

}

// Uses innerEllipse and outerEllipse and dirWithGreen to
//	scan the binary code along the external parts of the marker
void MarkerCC1::scanEllipses(Mat &srcCC)
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

	int bitIdx = 0;
	// Inner scanning ellipse (16 points)
	for(float directionAngle=0.0; directionAngle<359.0; directionAngle += 22.5)
	{
		Point location = getEllipsePointInDirection(baseEllipse,directionAngle,2.5, srcCC);
		rawMarkerIDBitCC[bitIdx]=srcCC.at<uchar>(location);
		bitLocations[bitIdx] = location;
		bitIdx++;
		if (verboseImage!=NULL && ConfigManager::Current()->verboseEllipseScanning)
		{
			circle(*verboseImage,location,3,Scalar(255,255,255));
/*			if (directionAngle==0.0)	// Indicate start direction
			{
				line(*verboseImage,markerCenter,location,Scalar(0,0,0));
			} */
		}
	}
	// Outer scanning ellpise (32 points)
	for(float directionAngle=0.0; directionAngle<359.0; directionAngle += 11.25)
	{
		Point location = getEllipsePointInDirection(baseEllipse,directionAngle,3.5, srcCC);

		rawMarkerIDBitCC[bitIdx]=srcCC.at<uchar>(location);
		bitLocations[bitIdx] = location;
		bitIdx++;
		if (verboseImage!=NULL && ConfigManager::Current()->verboseEllipseScanning)
		{
			circle(*verboseImage,location,3,Scalar(255,255,255));
		}
	}
}

// srcCC is only used to check wether location is inside the image
Point MarkerCC1::getEllipsePointInDirection(RotatedRect baseEllipse,float directionAngle,float distanceMultiplier, Mat &srcCC)
{
	// TODO: this takes lots of time now...

	// Calculating distance from origin based on an unrotated ellipse
	float x = baseEllipse.size.width/2 * sin( directionAngle /180.0*CV_PI );
	float y = baseEllipse.size.height/2 * cos( directionAngle /180.0*CV_PI );
	float distance = sqrt( x*x + y*y ) * distanceMultiplier;

	// Angle is in degrees, starting from north and increasing clockwise.
	int finalX = (int)(baseEllipse.center.x + distance * sin( (baseEllipse.angle + directionAngle) /180.0*CV_PI));
	int finalY = (int)(baseEllipse.center.y - distance * cos( (baseEllipse.angle + directionAngle) /180.0*CV_PI));

	// Do not return locations outside the image
	if (finalX < 0) finalX=0;
	if (finalX >= srcCC.cols) finalX=srcCC.cols-1;
	if (finalY < 0) finalY=0;
	if (finalY >= srcCC.rows) finalY=srcCC.rows-1;

	return Point(finalX,finalY);
}

// Reads pixels along a line to find borders of various color areas
// Return: false for immediate reject, True: may be valid marker...
bool MarkerCC1::findBordersAlongLine(Mat &srcCC, int dir)
{
	CvPoint endpoint = getEndPoint(center.x, center.y, scanDistance, dir);
	
	// Meanwhile, also find inner and outer borders of RED area
	// Fills RedInnerBorders[dir] and RedOuterBorders[]
	int currentAreaColorCode = COLORCODE_INITIAL;
	bool isDirectionValid = false;	// True if BLUE and RED area has been found. False otherwise.

	// Internal valiables to track changes and last locations
	uchar colorValue = -1;		// Color code for current pixel
	uchar prevColorValue = -1;	// Used to suppress 1 pixel errors
	Point prevPoint;			// Location of previous point
	int invalidPixelColorCounter = 0;		// Couting the number of pixels with invalid color. Beyond MAXINVALIDCOLORNUM, we do not search for green anymore...

	// Scan along the given line and store the color codes (using hue2codeLUT)
	LineIterator lineIt(srcCC,center,endpoint);
	int length = lineIt.count;
	for(int i=0; i<length; i++,lineIt++)
	{
		colorValue = *lineIt.ptr;

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
					if (ConfigManager::Current()->verboseTxt_LineRejectionReason)
					{
						cout << "MarkerCC1.findBordersAlongLine() LineRejection reson: first color is not BLU, dir="<<dir<<endl;
					}

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
					if (ConfigManager::Current()->verboseTxt_LineRejectionReason)
					{
						cout << "MarkerCC1.findBordersAlongLine() LineRejection reson: found GRN after BLU, dir="<<dir<<endl;
					}
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
					if (ConfigManager::Current()->verboseTxt_LineRejectionReason)
					{
						cout << "MarkerCC1.findBordersAlongLine() LineRejection reson: found BLU after RED, dir="<<dir<<endl;
					}
					return false;	// Immediate reject
					break;
				case COLORCODE_NONE:
				case COLORCODE_BLK:
				case COLORCODE_WHT:
					// White, black or unrecognized color means the end of the red area.
					RedOuterBorders[dir] = prevPoint;	// The previous point was already outside the RED area.
					isDirectionValid = true;	// Marker direction is now valid.
					currentAreaColorCode = COLORCODE_NONE;
					break;
				}
				break;
			case COLORCODE_GRN:
				break;
			}
		}

		// Show small circle with color representing current finite state machine state
		if (verboseImage != NULL && ConfigManager::Current()->verboseLineScanning )
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

	return isDirectionValid;
}

CvPoint MarkerCC1::getEndPoint(int x, int y, int distance, int dir)
{
	CV_Assert(dir>=0 && dir<8);

	const int dirXMul[] = {0,1,1,1,0,-1,-1,-1};
	const int dirYMul[] = {-1,-1,0,1,1,1,0,-1};

	// TODO: is is smart enough not to go outside the image?
	CvPoint endpoint = cvPoint(
		x + dirXMul[dir]*distance,
		y + dirYMul[dir]*distance);

	return endpoint;
}

// After the raw bits are read (oversampled),
//	this method calculates the final marker ID and validates it.
void MarkerCC1::validateAndConsolidateMarkerCode()
{
	// Get binary bits and finds green color
	int rawbits[48];
	// helper variables to find best green index
	int firstGreenIdx = -1;
	int lastGreenIdx = -1;
	int greenRunLength = 0;	// How many greens are there after each other?

	int greenIdx = -1;	// Index of last green location

	// --- Convert the color code array into a bit array (only 0 and 1)
	// --- Meanwhile find the green location (start direction)
	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		cout << "rawBits:";
	}

	bool isGreen;
	for (int bitIdx=0; bitIdx<48; bitIdx++)
	{
		isGreen = false;
		int colorCode = rawMarkerIDBitCC[bitIdx];
		// White is 1, other colors are 0.
		rawbits[bitIdx] = colorCode==COLORCODE_WHT ? 1 : 0;

		// Search for green color (only on outer ellipse)
		if (colorCode == COLORCODE_GRN && bitIdx>=16)
		{
			greenRunLength++;
			isGreen = true;
		}
		else
		{
			greenRunLength=0;
		}

		if (greenRunLength > 1)	// We have a longer green sequence
		{
			firstGreenIdx = bitIdx - greenRunLength + 1;
			lastGreenIdx = bitIdx;
		}

		if (ConfigManager::Current()->verboseMarkerCodeValidation )
		{
			if (isGreen)
			{
				if (greenRunLength>1)
				{
					cout << "G";	// longer green sequence
				}
				else
				{
					cout << "g";	// short green sequence
				}
			}
			else
			{
				cout << rawbits[bitIdx];
			}
		}

	}
	if (lastGreenIdx >= firstGreenIdx)
	{
		greenIdx = (lastGreenIdx + firstGreenIdx) / 2;
	}
	else	// May wrap around!
	{
		greenIdx = firstGreenIdx;	// Green must be at the wrap around location...
	}

	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		cout << ", GRN@(" << firstGreenIdx << "-" << lastGreenIdx << ")->" << greenIdx << endl;
	}

	// --- Reorder bits to start from the green location
	if (greenIdx<16)	// If green is not on the outer ellipse, marker ID cannot be valid.
	{
		isValid = false;
		majorMarkerID=0;
		minorMarkerID=0;
		return;
	}

	int realBitIdxInner[4];	// Bit indices (for rawbits[]) used for inner code
	int realBitIdxOuter[8];	// Bit indices (for rawbits[]) used for outer code
	int finalInnerBits[4];	// Final bits of inner code
	int finalOuterBits[8];	// Final bits of outer code
	unsigned int innerCode = 0;	// Numerical value of inner code
	unsigned int outerCode = 0;	// Numerical value of outer code
	// Calculate outer code
	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		cout << "GrnIdx:" << greenIdx << ", Outer code: ";
	}
	for(int i=0; i<8; i++)
	{
		realBitIdxOuter[i] = 16 + ((greenIdx-16)+4*i) % 32;
		finalOuterBits[i] = rawbits[realBitIdxOuter[i]];
		if (ConfigManager::Current()->verboseMarkerCodeValidation )
		{
			cout << finalOuterBits[i];
		}
		outerCode |= finalOuterBits[i];
		if (i<7)
			outerCode <<= 1;
	}
	// Verbose outer code reading and green location
	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		// Show locations and corresponding values really used
		for (int i=0; i<8; i++)
		{
			Point location = bitLocations[realBitIdxOuter[i]];
			if (finalOuterBits[i])	// This bit is 1: white circle with black outer edge
			{
				circle(*verboseImage,location,5,Scalar(255,255,255),3);
				circle(*verboseImage,location,10,Scalar(0,0,0),1);
			}
			else	// This bit is 0: black circle with white outer edge
			{
				circle(*verboseImage,location,5,Scalar(0,0,0),3);
				circle(*verboseImage,location,10,Scalar(255,255,255),1);
			}
		}

		// Little green circle with white outer edge indicates green location
		Point greenLocation = bitLocations[greenIdx];
		circle(*verboseImage,greenLocation,3,Scalar(0,255,0),2);
		circle(*verboseImage,greenLocation,4,Scalar(255,255,255),1);
	}


	// Calculate inner code
	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		cout << ", inner code: ";
	}
	for(int i=0; i<4; i++)
	{
		// Calculate index from index for corresponding direction
		//	on the outer ellipse (twice as many bits, indexing of outer
		//	ellipse begins with 16).
		//realBitIdxInner[i] = ((greenIdx-16)+4*i) % 16;

		realBitIdxInner[i] = ((realBitIdxOuter[2*i]-16)/2 + 2) % 16;
		finalInnerBits[i] = rawbits[realBitIdxInner[i]];
		if (ConfigManager::Current()->verboseMarkerCodeValidation )
		{
			cout << finalInnerBits[i];
		}
		innerCode |= finalInnerBits[i];
		if (i<3)
			innerCode <<= 1;
	}
	// Verbose outer code reading
	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		// Show locations and corresponding values really used
		for (int i=0; i<4; i++)
		{
			Point location = bitLocations[realBitIdxInner[i]];
			if (finalInnerBits[i])	// This bit is 1
			{
				circle(*verboseImage,location,5,Scalar(255,255,255),3);
				circle(*verboseImage,location,10,Scalar(0,0,0),1);
			}
			else	// This bit is 0
			{
				circle(*verboseImage,location,5,Scalar(0,0,0),3);
				circle(*verboseImage,location,10,Scalar(255,255,255),1);
			}
		}
	}
	// Verbose codes
	if (ConfigManager::Current()->verboseMarkerCodeValidation )
	{
		cout << ", iCode=" << innerCode << " oCode=" << outerCode << endl;
	}

	majorMarkerID = innerCode;
	minorMarkerID = outerCode;

	// Is this a valid code?
	isValid = markerLocator->validateMarkerID(majorMarkerID, minorMarkerID);
}

void MarkerCC1::exportToTextStream(ostream *stream)
{
	(*stream) << "MCC2 X:" << center.x << ",Y:" << center.y <<
			",isCValid:" << isCenterValid << ",isIDValid:" << isValid <<
			",ID:" << majorMarkerID << "-" << minorMarkerID;
}

