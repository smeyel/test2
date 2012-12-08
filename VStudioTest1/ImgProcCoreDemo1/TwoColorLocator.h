#ifndef __TWOCOLORLOCATOR_H_
#define __TWOCOLORLOCATOR_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "FastColorFilter.h"

using namespace cv;

typedef enum _Location { Top, Bot, Left, Right } LocationEnum;

class TwoColorLocator
{
public:
	// Constructors
	TwoColorLocator();

	void findInitialRects(Mat &redMask, Mat &blueMask);	// Works on uchar masks with values 0 and 255
	void consolidateRects(Mat &srcCC);

	// Storage of results, cleared before every "apply()"
	std::list<CvRect> resultRectangles;		// final result created by consolidateRects

	// Image for subresult visualization
	Mat *verboseImage;

private:
	// Internal storage for initial rectangle candidates
	std::list<CvRect> initialRectangles;	// created by findInitialRects

	// Updates the size of a rectangle by scanning the image from the center in
	//	4 directions and finding the borders.
	bool updateRectToRealSize(Mat &srcCC, CvRect &newRect, Mat *verboseImage);

	// Goes along a line with a given (inner) color and returns the distance of the "border color" pixels. -1 indicates invalid situation.
	int findColorAlongLine(Mat &srcCC, Point startPoint, Point endPoint, uchar innerColorCode, uchar borderColorCode,Mat *verboseImage);

	// Calculates the occurrance numbers per row and column using the provided integral image.
	void getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax);
	// For verbose: draws results of getOccurranceNumbers on the verbose image.
	void drawValuesOnMargin(Mat &img, int *values, int valueNum,int scalingDivider, Scalar color, LocationEnum loc);
	// Generates the candidate rectangles using the per row/column occurrance numbers and a given threshold.
	void getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax, double thresholdRate, std::list<CvRect> &resultRectangles, Mat *verboseImage);
};

#endif
