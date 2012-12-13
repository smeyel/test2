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
#define MININNERPIXELNUM	2	// Number of inner (blue) pixels we need to accept the border (red) pixels after it (rect center should containt inner color)
#define MINRECTINTEGRAL 1		// Minimum of integral image sum of a rectangle to be candidate (used with overlapMask)

using namespace cv;
using namespace TwoColorCircleMarker;

TwoColorLocator::TwoColorLocator()
{
	verboseImage = NULL;
}

void TwoColorLocator::findInitialRectsFromOverlapMask(Mat &overlapMask)
{
	// --- Calculate integral images of masks
	Mat integralMask;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::IntegralImages);
	integral(overlapMask,integralMask,CV_32S);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::IntegralImages);

	// --- Calculating occurrance numbers for rows and colums using the integral images
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ProcessIntegralImages);
	int *rowOccNums = new int[overlapMask.rows];
	int *colOccNums = new int[overlapMask.cols];
	int rowmax = 0;
	int colmax = 0;
	// H1 mask
	getOccurranceNumbers(integralMask,rowOccNums,colOccNums,rowmax,colmax);

	// --- Verbose
	// Pixel numbers on the margins
	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,rowOccNums,overlapMask.rows,rowmax/50,Scalar(0,0,255),Right);
		drawValuesOnMargin(*verboseImage,colOccNums,overlapMask.cols,colmax/50,Scalar(0,0,255),Bot);
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ProcessIntegralImages);

	// --- Generate initial rectanges from two-color co-occurrances
	initialRectangles.clear();

	getMarkerCandidateRectanges(rowOccNums, colOccNums, overlapMask.rows, overlapMask.cols,
		rowmax, colmax, 0.0, initialRectangles, &integralMask, verboseImage);

	delete rowOccNums;
	delete colOccNums;
}

void TwoColorLocator::findInitialRects(Mat &borderColorMask, Mat &innerColorMask)
{
	CV_Assert(borderColorMask.rows == innerColorMask.rows);
	CV_Assert(borderColorMask.cols == innerColorMask.cols);

	// --- Calculate integral images of masks
	Mat h1integral, h2integral;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::IntegralImages);
	integral(borderColorMask,h1integral,CV_32S);
	integral(innerColorMask,h2integral,CV_32S);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::IntegralImages);

	// --- Calculating occurrance numbers for rows and colums using the integral images
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ProcessIntegralImages);
	int *h1rowOccNums = new int[borderColorMask.rows];
	int *h1colOccNums = new int[borderColorMask.cols];
	int *h2rowOccNums = new int[innerColorMask.rows];
	int *h2colOccNums = new int[innerColorMask.cols];
	int rowmax = 0;
	int colmax = 0;
	// H1 mask
	getOccurranceNumbers(h1integral,h1rowOccNums,h1colOccNums,rowmax,colmax);
	// H2 mask
	getOccurranceNumbers(h2integral,h2rowOccNums,h2colOccNums,rowmax,colmax);

	// Merging two color occurrance numbers
	int rownum = innerColorMask.rows;
	int colnum = innerColorMask.cols;
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

	// --- Verbose
	// Pixel numbers on the margins
	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,h1rowOccNums,borderColorMask.rows,rowmax/50,Scalar(0,0,255),Right);
		drawValuesOnMargin(*verboseImage,h1colOccNums,borderColorMask.cols,colmax/50,Scalar(0,0,255),Bot);
		drawValuesOnMargin(*verboseImage,h2rowOccNums,innerColorMask.rows,rowmax/50,Scalar(255,0,0),Left);
		drawValuesOnMargin(*verboseImage,h2colOccNums,innerColorMask.cols,colmax/50,Scalar(255,0,0),Top);
	}
	// Intersection indication on the margins
	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,mergedRowOccNums,innerColorMask.rows,mergedRowMax/50,Scalar(0,255,255),Right);
		drawValuesOnMargin(*verboseImage,mergedColOccNums,innerColorMask.cols,mergedColMax/50,Scalar(0,255,255),Bot);
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ProcessIntegralImages);

	delete h1rowOccNums;
	delete h1colOccNums;
	delete h2rowOccNums;
	delete h2colOccNums;

	// --- Generate initial rectanges from two-color co-occurrances
	initialRectangles.clear();

	getMarkerCandidateRectanges(mergedRowOccNums, mergedColOccNums, rownum, colnum,
		mergedRowMax, mergedColMax, 0.2, initialRectangles, NULL, verboseImage);

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
			if (verboseImage!=NULL && ConfigManager::Current()->verboseRectConsolidationResults)
			{
				rectangle(*verboseImage,Rect(newRect),Scalar(0,255,0));
			}

			// Add to resultRectangles
			resultRectangles.push_back(newRect);
			resultRectNum++;
		}
	}

	if (ConfigManager::Current()->verboseTxt_RectConsolidationSummary)
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
	// Horizontal scan
	int leftLength = findColorAlongLine(srcCC, center, Point(center.x-MAXSCANDISTANCE, center.y), COLORCODE_BLU, COLORCODE_RED, verboseImage);
	int rightLength = findColorAlongLine(srcCC, center, Point(center.x+MAXSCANDISTANCE, center.y), COLORCODE_BLU, COLORCODE_RED, verboseImage);
	if (leftLength != -1 && rightLength != -1)
	{
		// update center location
		center.x = center.x + (rightLength - leftLength)/2;
	}
	else
	{
		if (ConfigManager::Current()->verboseTxt_RectConsolidation)
		{
			cout << "TwoColorLocator::updateRectToRealSize(): horizontal -> REJECT" << endl;
		}
		return false;
	}


	int topLength = findColorAlongLine(srcCC, center, Point(center.x, center.y-MAXSCANDISTANCE), COLORCODE_BLU, COLORCODE_RED, verboseImage);
	int bottomLength = findColorAlongLine(srcCC, center, Point(center.x, center.y+MAXSCANDISTANCE), COLORCODE_BLU, COLORCODE_RED, verboseImage);

	if (ConfigManager::Current()->verboseTxt_RectConsolidation)
	{
		cout << "Rect consolidation: x" << newRect.x << " y" << newRect.y << " w" << newRect.width << " h" << newRect.height;
	}
	// Update rect sizes (if the borders were really found)
	if (topLength != -1 && bottomLength != -1)
	{
		newRect.x = center.x-leftLength;
		newRect.y = center.y-topLength;
		newRect.width = leftLength + rightLength;
		newRect.height = topLength + bottomLength;
		if (ConfigManager::Current()->verboseTxt_RectConsolidation)
		{
			cout << " -> x" << newRect.x << " y" << newRect.y << " w" << newRect.width << " h" << newRect.height << endl;
		}
		return true;
	}
	else
	{
		if (ConfigManager::Current()->verboseTxt_RectConsolidation)
		{
			cout << " -> REJECT" << endl;
		}
		return false;
	}

}

// Returns distance of first given color along the given line, or -1
int TwoColorLocator::findColorAlongLine(Mat &srcCC, Point startPoint, Point endPoint, uchar innerColorCode, uchar borderColorCode,Mat *verboseImage)
{
	// Internal valiables to track changes and last locations
	uchar colorValue = COLORCODE_INITIAL;		// Color code for current pixel
	uchar prevColorValue = COLORCODE_INITIAL;	// Used to suppress 1 pixel errors
	int innerPixelNum = 0;		// Count blue pixels. There must be at least MINBLUEPIXELNUM to accept
	int invalidPixelColorCounter = 0;		// Couting the number of pixels with invalid color. Beyond MAXINVALIDCOLORNUM, we do not search for green anymore...

	// Scan along the given line and store the color codes (using hue2codeLUT)
	LineIterator lineIt(srcCC,startPoint,endPoint);
	int length = lineIt.count;
	for(int i=0; i<length; i++,lineIt++)
	{
		colorValue = *lineIt.ptr;

		if (colorValue != innerColorCode && colorValue != borderColorCode)
		{
			invalidPixelColorCounter++;
			if (invalidPixelColorCounter>MAXINVALIDCOLORNUM)
			{
				// No more valuable pixels. (May already be OK!)
				break;
			}
		}

		if (colorValue == innerColorCode)
		{
			innerPixelNum++;
		}

		// Stop if current and previous colors are both RED
		// Continue otherwise
		if (colorValue == borderColorCode && prevColorValue == borderColorCode)
		{
			// If there were insufficient blue pixels, we return -1
			if (innerPixelNum < MININNERPIXELNUM)
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

// Creates candidate rectangles from the row and column sums
// If integralMask is given (for cases with overlapMask), integral of candidate rectangles are calculated checked.
void TwoColorLocator::getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax,
	double thresholdRate, std::list<CvRect> &resultRectangles, Mat *integralMask, Mat *verboseImage)
{
	CV_Assert(sizeof(int) == 4);	// Used to handle integralMask (assuming type int matrix)
//	CV_Assert(rownum == verboseImage->rows);
//	CV_Assert(colnum == verboseImage->cols);
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::GetCandidateRectangles);

	// Thresholding for 30%
	int rowValThreshold = rowMax * thresholdRate;
	int colValThreshold = colMax * thresholdRate;
	if (thresholdRate == 0.0)	// Used with overlap mask, where a single line is also very important
	{
		rowValThreshold = 1;
		colValThreshold = 1;
	}

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

			// If integralMask is given, integral of rect is checked
			if (integralMask != NULL)
			{
				
				int integralTopLeft = integralMask->at<int>(y,x);
				int integralTopRight = integralMask->at<int>(y,x+width);
				int integralBottomLeft = integralMask->at<int>(y+height,x);
				int integralBottomRight = integralMask->at<int>(y+height,x+width);

				int sum = integralTopLeft + integralBottomRight - integralTopRight - integralBottomLeft;

				if (sum < MINRECTINTEGRAL)
				{
					if (ConfigManager::Current()->verboseTwoColorLocatorIntegralReject)
					{
						cout << "Rect reject due to low integral: " << sum << endl;
					}
					// Continue without adding the rectangle to the candidates
					continue;
				}
			}

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
