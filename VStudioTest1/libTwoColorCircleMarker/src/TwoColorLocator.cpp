#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "TwoColorLocator.h"

#include <assert.h>

#include "TimeMeasurementCodeDefines.h"

#include "Logger.h"

// Maximal distance we look for the border of a rectangle
#define MAXSCANDISTANCE 100
#define MAXINVALIDCOLORNUM 20	// Number of non-blue pixels we pass while searching for red...
#define MININNERPIXELNUM	2	// Number of inner (blue) pixels we need to accept the border (red) pixels after it (rect center should containt inner color)
#define MINRECTINTEGRAL 1		// Minimum of integral image sum of a rectangle to be candidate (used with overlapMask)

using namespace cv;
using namespace std;
using namespace TwoColorCircleMarker;
using namespace Logging;

#define LOG_TAG "SMEyeL::TwoColorLocator"

// Config manager
bool TwoColorLocator::ConfigManager::readConfiguration(CSimpleIniA *ini)
{
	verboseRectConsolidationCandidates = ini->GetBoolValue("TwoColorLocator","verboseRectConsolidationCandidates",false,NULL);
	verboseRectConsolidationResults = ini->GetBoolValue("TwoColorLocator","verboseRectConsolidationResults",false,NULL);
	verboseTxt_RectConsolidation = ini->GetBoolValue("TwoColorLocator","verboseTxt_RectConsolidation",false,NULL);
	verboseTxt_RectConsolidationSummary = ini->GetBoolValue("TwoColorLocator","verboseTxt_RectConsolidationSummary",false,NULL);
	return true;	// Successful
}

TwoColorLocator::TwoColorLocator()
{
	verboseImage = NULL;
}

void TwoColorLocator::init(char *configFileName)
{
	configManager.init(configFileName);
}

// Called externally after FastColorFilter has detected its candidate rectangles
void TwoColorLocator::consolidateFastColorFilterRects(Rect* candidateRects, int candidateRectNum, Mat &srcCC)
{
	int initialRectNum = 0;
	int resultRectNum = 0;
	resultRectangles.clear();
	if (candidateRectNum<=0)
	{
		return;	// Nothing to do.
	}

	// Load candidates into initialRectangles
	std::list<Rect> candidateRectList;
	candidateRectList.clear();	// obsolete
	for(int i=0; i<candidateRectNum; i++)
	{
		candidateRectList.push_front(candidateRects[i]);

		// Verbose candidate rectangles
		if (verboseImage!=NULL && configManager.verboseRectConsolidationCandidates)
		{
			rectangle(*verboseImage,candidateRects[i],Scalar(200,255,200));
		}
	}

	// Go along every candidate rectangle,
	//	find real limits in 4 directions and
	//	create new rectangle using the new sizes.
	// Finally, skip every rectangle with a center inside
	//	the new rectangle.
	while (!candidateRectList.empty())	// As long as there are rectangles to consolidate
	{
		initialRectNum++;
		// Copy newRect (we will need a copy later anyway)
		Rect newRect = candidateRectList.front();
		candidateRectList.pop_front();

		// Check for overlapping with already present result rectangles
		bool isOverlapping = false;
		list<Rect>::iterator it;
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
			if (verboseImage!=NULL && configManager.verboseRectConsolidationResults)
			{
				rectangle(*verboseImage,newRect,Scalar(0,255,0));
			}

			// Add to resultRectangles
			resultRectangles.push_back(newRect);
			resultRectNum++;
		}
	}

	if (configManager.verboseTxt_RectConsolidationSummary)
	{
		Logger::log(Logger::LOGLEVEL_VERBOSE, LOG_TAG, "Rect consolidation effect: %d rect -> %d\n", initialRectNum, resultRectNum);
	}
}

// Returns true if rect seems to be valid
bool TwoColorLocator::updateRectToRealSize(Mat &srcCC, Rect &newRect, Mat *verboseImage)
{

	// for collecting log data
	std::stringstream logMsg;


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
		if (configManager.verboseTxt_RectConsolidation)
		{
			Logger::log(Logger::LOGLEVEL_VERBOSE, LOG_TAG, "TwoColorLocator::updateRectToRealSize(): horizontal -> REJECT\n");
		}
		return false;
	}

	int topLength = findColorAlongLine(srcCC, center, Point(center.x, center.y-MAXSCANDISTANCE), COLORCODE_BLU, COLORCODE_RED, verboseImage);
	int bottomLength = findColorAlongLine(srcCC, center, Point(center.x, center.y+MAXSCANDISTANCE), COLORCODE_BLU, COLORCODE_RED, verboseImage);

	if (configManager.verboseTxt_RectConsolidation)
	{
		logMsg << "Rect consolidation: x" << newRect.x << " y" << newRect.y << " w" << newRect.width << " h" << newRect.height;
	}
	// Update rect sizes (if the borders were really found)
	if (topLength != -1 && bottomLength != -1)
	{
		newRect.x = center.x-leftLength;
		newRect.y = center.y-topLength;
		newRect.width = leftLength + rightLength;
		newRect.height = topLength + bottomLength;
		if (configManager.verboseTxt_RectConsolidation)
		{
			logMsg << " -> x" << newRect.x << " y" << newRect.y << " w" << newRect.width << " h" << newRect.height << endl;
			Logger::log(Logger::LOGLEVEL_VERBOSE, LOG_TAG, logMsg.str().c_str());
			logMsg.str(""); // clear
		}
		return true;
	}
	else
	{
		if (configManager.verboseTxt_RectConsolidation)
		{
			logMsg << " -> REJECT" << endl;
			Logger::log(Logger::LOGLEVEL_VERBOSE, LOG_TAG, logMsg.str().c_str());
			logMsg.str(""); // clear
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
