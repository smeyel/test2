#ifndef _MARKERSEARCH_H_
#define _MARKERSEARCH_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace cv;
using namespace std;

typedef struct _FilterThreshold
{
	double low, high;
} FilterThreshold;

void mouse_callback(int eventtype, int x, int y, int flags, void *param);
/*void filterBGRInHS(Mat *bgr,
	FilterThreshold hue1Mask, FilterThreshold hue2Mask, FilterThreshold satMask);
void getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax);
void drawValuesOnMargin(Mat &img, int *values, int valueNum,
	int scalingDivider, Scalar color, LocationEnum loc);
void getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax,
	double thresholdRate, std::list<CvRect> &resultRectangles, Mat *imgForDebug);*/
void processFrame(Mat *input, Mat *result);

#endif