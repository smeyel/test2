#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC1.h"

#include <assert.h>

#include "TimeMeasurementCodeDefines.h"
#include "ConfigManager.h"

// Maximal distance we look for the border of a rectangle
#define MAXSCANDISTANCE 100
#define MAXINVALIDCOLORNUM 20	// Number of non-blue pixels we pass while searching for red...
#define MINBLUEPIXELNUM	2		// Number of blue pixels we need to accept the red pixels after it (rect center should containt blue)

using namespace cv;

TwoColorLocator::TwoColorLocator()
{
	fastColorFilter = NULL;
	verboseImage = NULL;
}


void TwoColorLocator::applyOnCC(Mat &redMask, Mat &blueMask)
{
	assert(fastColorFilter!=NULL);

	if (verboseImage != NULL)
	{
		// Verbose image-nek az alapja az eredeti kep
		//srcCC.copyTo(*verboseImage);	// NE CSAPJUK FELUL, AMI MAR RAJTA VAN!
	}

	// --- Calculate integral images of masks
	Mat h1integral, h2integral;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::IntegralImages);
	integral(redMask,h1integral,CV_32S);
	integral(blueMask,h2integral,CV_32S);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::IntegralImages);

	CV_Assert(redMask.rows == blueMask.rows);
	CV_Assert(blueMask.cols == blueMask.cols);

	// --- Az integral image alapjan a sorok es oszopok pixelertek osszegeit szamoljuk ki minden maszkra
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ProcessIntegralImages);
	int *h1rowOccNums = new int[redMask.rows];
	int *h1colOccNums = new int[redMask.cols];
	int *h2rowOccNums = new int[blueMask.rows];
	int *h2colOccNums = new int[blueMask.cols];
	int rowmax = 0;
	int colmax = 0;
	// H1 mask
	getOccurranceNumbers(h1integral,h1rowOccNums,h1colOccNums,rowmax,colmax);
	// H2 mask
	getOccurranceNumbers(h2integral,h2rowOccNums,h2colOccNums,rowmax,colmax);

	// Peremen megjelenitjuk a pixel darabszamokat az egyes maszkok szerint
	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,h1rowOccNums,redMask.rows,rowmax/50,Scalar(0,0,255),Right);
		drawValuesOnMargin(*verboseImage,h1colOccNums,redMask.cols,colmax/50,Scalar(0,0,255),Bot);
		drawValuesOnMargin(*verboseImage,h2rowOccNums,blueMask.rows,rowmax/50,Scalar(255,0,0),Left);
		drawValuesOnMargin(*verboseImage,h2colOccNums,blueMask.cols,colmax/50,Scalar(255,0,0),Top);
	}

	// --- A ket maszk osszevonasa egybe (hol teljesul mindketto "peremfeltetele")
	int rownum = blueMask.rows;
	int colnum = blueMask.cols;
	int *mergedRowOccNums = new int[rownum];
	int *mergedColOccNums = new int[colnum];
	// Calculating minimum of two vectors (merging)
	int mergedRowMax = 0;
	int mergedColMax = 0;
	for(int i=0; i<rownum; i++)
	{
		mergedRowOccNums[i] = h1rowOccNums[i]<h2rowOccNums[i] ? h1rowOccNums[i] : h2rowOccNums[i];
		if (mergedRowMax<mergedRowOccNums[i])
			mergedRowMax = mergedRowOccNums[i];
	}
	for(int i=0; i<colnum; i++)
	{
		mergedColOccNums[i] = h1colOccNums[i]<h2colOccNums[i] ? h1colOccNums[i] : h2colOccNums[i];
		if (mergedColMax<mergedColOccNums[i])
			mergedColMax = mergedColOccNums[i];
	}

	delete h1rowOccNums;
	delete h1colOccNums;
	delete h2rowOccNums;
	delete h2colOccNums;

	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,mergedRowOccNums,blueMask.rows,mergedRowMax/50,Scalar(0,255,255),Right);
		drawValuesOnMargin(*verboseImage,mergedColOccNums,blueMask.cols,mergedColMax/50,Scalar(0,255,255),Bot);
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ProcessIntegralImages);

	initialRectangles.clear();

	getMarkerCandidateRectanges(mergedRowOccNums, mergedColOccNums, rownum, colnum,
		mergedRowMax, mergedColMax, 0.2, initialRectangles, verboseImage);

	delete mergedRowOccNums;
	delete mergedColOccNums;
}

// Called externally after apply to consolidate the initial rectangles.
void TwoColorLocator::consolidateRects(Mat &srcCC)
{
	int initialRectNum = 0;
	int resultRectNum = 0;
	resultRectangles.clear();
	if (initialRectangles.empty())
	{
		return;	// Nothing to do.
	}

	// Go along every initial rectangle,
	//	find real limits in 4 directions and
	//	create new rectangle using the new sizes.
	// Finally, skip every rectangle with a center inside
	//	the new rectangle.
	CvRect newRect;
	while (!initialRectangles.empty())	// As long as there are rectangles to consolidate
	{
		initialRectNum++;
		// Copy newRect (we will need a copy later anyway)
		CvRect newRect = initialRectangles.front();
		initialRectangles.pop_front();

		// Check for overlapping with already present result rectangles
		bool isOverlapping = false;
		list<CvRect>::iterator it;
		for ( it=resultRectangles.begin() ; it != resultRectangles.end(); it++ )
		{
			// is the center of newRect inside *it?
			Point center = Point(newRect.x + newRect.width / 2, newRect.y + newRect.height / 2);
			if (center.x >= it->x && center.x <= it->x+it->width && 
				center.y >= it->y && center.y <= it->y+it->height)
			{
				isOverlapping = true;
				break;
			}
		}
		if (isOverlapping)
		{
			// Jump to next initial rectangle, as this is overlapping with an already consolidated one.
			continue;
		}

		// Update rect to its real size
		if (updateRectToRealSize(srcCC, newRect, verboseImage))
		{
			// Verbose: show new rectangle
			if (verboseImage!=NULL && ConfigManager::Current()->verboseRectConsolidation)
			{
				rectangle(*verboseImage,Rect(newRect),Scalar(0,255,0));
			}

			// Add to resultRectangles
			resultRectangles.push_back(newRect);
			resultRectNum++;
		}
	}

	if (ConfigManager::Current()->verboseRectConsolidation)
	{
		cout << "Rect consolidation effect: " << initialRectNum << " rect -> " << resultRectNum << endl;
	}
}

// Returns true if rect seems to be valid
bool TwoColorLocator::updateRectToRealSize(Mat &srcCC, CvRect &newRect, Mat *verboseImage)
{
	// From the center, scan in every direction and find the first RED pixel
	Point center = Point(newRect.x+newRect.width/2, newRect.y+newRect.height/2);

	// TODO: does the iterator stop at the border of the image?
	int leftLength = findRedAlongLine(srcCC, center, Point(center.x-MAXSCANDISTANCE, center.y), verboseImage);
	int rightLength = findRedAlongLine(srcCC, center, Point(center.x+MAXSCANDISTANCE, center.y), verboseImage);
	int topLength = findRedAlongLine(srcCC, center, Point(center.x, center.y-MAXSCANDISTANCE), verboseImage);
	int bottomLength = findRedAlongLine(srcCC, center, Point(center.x, center.y+MAXSCANDISTANCE), verboseImage);

	if (ConfigManager::Current()->verboseRectConsolidation)
	{
		cout << "Rect consolidation: x" << newRect.x << " y" << newRect.y << " w" << newRect.width << " h" << newRect.height;
	}
	// Update rect sizes (if the borders were really found)
	if (leftLength != -1 && rightLength != -1 && topLength != -1 && bottomLength != -1)
	{
		newRect.x = center.x-leftLength;
		newRect.y = center.y-topLength;
		newRect.width = leftLength + rightLength;
		newRect.height = topLength + bottomLength;
		if (ConfigManager::Current()->verboseRectConsolidation)
		{
			cout << " -> x" << newRect.x << " y" << newRect.y << " w" << newRect.width << " h" << newRect.height << endl;
		}
		return true;
	}
	else
	{
		if (ConfigManager::Current()->verboseRectConsolidation)
		{
			cout << " -> REJECT" << endl;
		}
		return false;
	}

}

// Returns distance of first RED color along the given line, or -1
int TwoColorLocator::findRedAlongLine(Mat &srcCC, Point startPoint, Point endPoint, Mat *verboseImage)
{
	// Meanwhile, also find inner and outer borders of RED (value 1) area
	// Fills RedInnerBorders[dir] and RedOuterBorders[]
	int currentAreaColorCode = COLORCODE_INITIAL;
	bool isDirectionValid = false;	// True if RED area has been found. False otherwise.

	// Internal valiables to track changes and last locations
	uchar colorValue = -1;		// Color code for current pixel
	uchar prevColorValue = -1;	// Used to suppress 1 pixel errors
	int bluePixelNum = 0;		// Count blue pixels. There must be at least MINBLUEPIXELNUM to accept
//	Point prevPoint;			// Location of previous point
	int invalidPixelColorCounter = 0;		// Couting the number of pixels with invalid color. Beyond MAXINVALIDCOLORNUM, we do not search for green anymore...

//	bool isGreenInThisDirection = false;	// True if green color was found directly beyond the red area

	// Scan along the given line and store the color codes (using hue2codeLUT)
	LineIterator lineIt(srcCC,startPoint,endPoint);
	int length = lineIt.count;
	for(int i=0; i<length; i++,lineIt++)
	{
		colorValue = *lineIt.ptr;

/*		if (verboseImage != NULL && ConfigManager::Current()->verboseRectConsolidation)
		{
			Point pos = lineIt.pos();
			circle(*verboseImage,pos,3,Scalar(0,0,0));
		}*/

		if (colorValue != COLORCODE_BLU && colorValue != COLORCODE_RED)
		{
			invalidPixelColorCounter++;
			if (invalidPixelColorCounter>MAXINVALIDCOLORNUM)
			{
				// No more valuable pixels. (May already be OK!)
				break;
			}
		}

		if (colorValue == COLORCODE_BLU)
		{
			bluePixelNum++;
		}

		// Stop if current and previous colors are both RED
		// Continue otherwise
		if (colorValue == COLORCODE_RED && prevColorValue == COLORCODE_RED)
		{
			// If there were insufficient blue pixels, we return -1
			if (bluePixelNum < MINBLUEPIXELNUM)
			{
				return -1;	// Reject
			}
			// Return the distance we have searched for.
			// -1, as we are at the 2nd red pixel.
			return i-1;
		}

		prevColorValue = colorValue;
	}

	// If we arrived here, we did not find red.
	return -1;
}

// Az integral image peremosszegeit szamolja ki
void TwoColorLocator::getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax)
{
	// rowmax and colmax should not be reset as they might have other, already calculated values.

	// TODO: this method is slow for accessing the integral image points. Can be accelerated!
	int rowNumMinus1 = srcIntegral.rows-1;
	int colNumMinus1 = srcIntegral.cols-1;
	// Warning: last row and column is omitted in result...
	for(int row=0; row<rowNumMinus1; row++)
	{
		int current = srcIntegral.at<int>(row, colNumMinus1);
		int next = srcIntegral.at<int>(row+1, colNumMinus1);
		int diff = (next - current) / 255;	// Have to divide, as the mask is 0/255 binary mask!
		rowOccNums[row] = diff;
		if (rowmax < diff)
			rowmax=diff;
	}
	for(int col=0; col<colNumMinus1; col++)
	{
		int current = srcIntegral.at<int>(rowNumMinus1, col);
		int next = srcIntegral.at<int>(rowNumMinus1, col+1);
		int diff = (next - current) / 255; 	// Have to divide, as the mask is 0/255 binary mask!
		colOccNums[col] = diff;
		if (colmax < diff)
			colmax=diff;
	}
}

// Draws the integral image margin sums on the margins of the image
void TwoColorLocator::drawValuesOnMargin(Mat &img, int *values, int valueNum,
	int scalingDivider, Scalar color, LocationEnum loc)
{
	if (scalingDivider==0)
	{
		return;
	}

	for(int i=0; i<valueNum; i++)
	{
		int value = values[i] / scalingDivider;
		CvPoint start, end;
		switch(loc)
		{
		case Top:
			start = cvPoint(i,0);
			end = cvPoint(i,value);
			break;
		case Bot:
			start = cvPoint(i,img.rows-1);
			end = cvPoint(i,img.rows-1-value);
			break;
		case Left:
			start = cvPoint(0,i);
			end = cvPoint(value,i);
			break;
		case Right:
			start = cvPoint(img.cols-1,i);
			end = cvPoint(img.cols-1-value,i);
			break;
		}

		line(img,start,end,color);
	}
}

// A peremosszegekbol threshold alapjan letrehozza az eselyes negyzetek listjajat.
void TwoColorLocator::getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax,
	double thresholdRate, std::list<CvRect> &resultRectangles, Mat *verboseImage)
{
//	CV_Assert(rownum == verboseImage->rows);
//	CV_Assert(colnum == verboseImage->cols);
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::GetCandidateRectangles);

	// Thresholding for 30%
	int rowValThreshold = rowMax * thresholdRate;
	int colValThreshold = colMax * thresholdRate;

	std::list<Scalar> rowIntervalList;
	int prev = 0;
	int currentIntervalBegin = 0;
	for (int i=0; i<rownum; i++)
	{
		if (rowVals[i] >= rowValThreshold)
		{
			// Now above threshold
			if (prev < rowValThreshold)
			{
				// previously below threshold, now entering new interval
				currentIntervalBegin = i;
			}
		}
		else
		{
			// Now below threshold
			if (prev >= rowValThreshold)
			{
				// previously above threshold, now exiting new interval
				Scalar newInterval = Scalar(currentIntervalBegin, i);
				rowIntervalList.push_back(newInterval);

				if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
				{
					rectangle(*verboseImage,cvPoint(0,currentIntervalBegin),cvPoint(20,i),Scalar(0,255,255));
				}
			}
		}

		prev = rowVals[i];
	}

	std::list<Scalar> colIntervalList;
	prev = 0;
	currentIntervalBegin = 0;
	for (int i=0; i<colnum; i++)
	{
		if (colVals[i] >= colValThreshold)
		{
			// Now above threshold
			if (prev < colValThreshold)
			{
				// previously below threshold, now entering new interval
				currentIntervalBegin = i;
			}
		}
		else
		{
			// Now below threshold
			if (prev >= colValThreshold)
			{
				// previously above threshold, now exiting new interval
				Scalar newInterval = Scalar(currentIntervalBegin, i);
				colIntervalList.push_back(newInterval);

				if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
				{
					rectangle(*verboseImage,cvPoint(currentIntervalBegin,0),cvPoint(i,20),Scalar(0,255,255));
				}
			}
		}

		prev = colVals[i];
	}

	// Combine intervals into rectangles
	for (	std::list<Scalar>::iterator rowIt = rowIntervalList.begin();
			rowIt != rowIntervalList.end();
			rowIt++ )
	{
		int y = (*rowIt)[0];
		int height = (*rowIt)[1] - y;
		for (	std::list<Scalar>::iterator colIt = colIntervalList.begin();
				colIt != colIntervalList.end();
				colIt++ )
		{
			int x = (*colIt)[0];
			int width = (*colIt)[1] - x;

			CvRect newRect = cvRect(x,y,width,height);

			resultRectangles.push_back(newRect);

			// visualize
			if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
			{
				rectangle(*verboseImage,newRect,Scalar(0,150,0));
			}
		}
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::GetCandidateRectangles);
}
