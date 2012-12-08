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
	FastColorFilter *fastColorFilter;

	void apply(Mat &src, Mat &resultMask);

	// Constructors
	TwoColorLocator();

//	void apply(Mat &src);
	void applyOnCC(Mat &redMask, Mat &blueMask);
	void consolidateRects(Mat &srcCC);

	// Storage of results, cleared before every "apply()"
	std::list<CvRect> resultRectangles;		// final result created by consolidateRects

	// Image for subresult visualization
	Mat *verboseImage;

private:
	std::list<CvRect> initialRectangles;	// created by applyOnCC

	bool updateRectToRealSize(Mat &srcCC, CvRect &newRect, Mat *verboseImage);
	int findRedAlongLine(Mat &srcCC, Point startPoint, Point endPoint, Mat *verboseImage);

	// Az integral image peremosszegeit szamolja ki
	void getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax);
	// Verbose-hoz: integral image peremosszegek a margokon
	void drawValuesOnMargin(Mat &img, int *values, int valueNum,int scalingDivider, Scalar color, LocationEnum loc);
	// Verbose-hoz: A peremosszegekbol threshold alapjan letrehozza az eselyes negyzetek listjajat.
	void getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax, double thresholdRate, std::list<CvRect> &resultRectangles, Mat *verboseImage);
};

#endif
