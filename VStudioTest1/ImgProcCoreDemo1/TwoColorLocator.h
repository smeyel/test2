#ifndef __TWOCOLORLOCATOR_H_
#define __TWOCOLORLOCATOR_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorFilter.h"

using namespace cv;

typedef enum _Location { Top, Bot, Left, Right } LocationEnum;

class TwoColorLocator
{
private:
	TwoColorFilter internalTwoColorFilter;
public:
	TwoColorFilter *twoColorFilter;
	bool verbose;

	void apply(Mat &src, Mat &resultMask);

	// Constructors
	TwoColorLocator();

	void apply(Mat &src);

	// Storage of results, cleared before every "apply()"
	std::list<CvRect> resultRectangles;


	// Image for subresult visualization
	Mat verboseImage;

private:
	// Az integral image peremosszegeit szamolja ki
	void getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax);
	// Verbose-hoz: integral image peremosszegek a margokon
	void drawValuesOnMargin(Mat &img, int *values, int valueNum,int scalingDivider, Scalar color, LocationEnum loc);
	// Verbose-hoz: A peremosszegekbol threshold alapjan letrehozza az eselyes negyzetek listjajat.
	void getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax, double thresholdRate, std::list<CvRect> &resultRectangles, bool verbose, Mat *verboseImage);
};

#endif
