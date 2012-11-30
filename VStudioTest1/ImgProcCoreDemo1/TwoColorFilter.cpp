#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
//#include "BandpassFilter.h"
//#include "LowpassFilter.h"
#include "TwoColorFilter.h"

using namespace cv;

TwoColorFilter::TwoColorFilter()
{
	hue1Filter = &internalH1Filter;
	hue2Filter = &internalH2Filter;
	satFilter = &internalSatFilter;
}

// Prepares default filters (pointers may be redirected...)
TwoColorFilter::TwoColorFilter(double sTh, double h1Low, double h1High, double h2Low, double h2High)
{
	internalSatFilter.threshold = sTh;
	internalH1Filter.lowThreshold = h1Low;
	internalH1Filter.highThreshold = h1High;
	internalH2Filter.lowThreshold = h2Low;
	internalH2Filter.highThreshold = h2High;

	hue1Filter = &internalH1Filter;
	hue2Filter = &internalH2Filter;
	satFilter = &internalSatFilter;
}

void TwoColorFilter::apply(Mat &srcBGR, Mat &resultHue1Mask, Mat &resultHue2Mask)
{
	// Extract hue channel
	cvtColor(srcBGR, imgHSV, CV_BGR2HSV);
	h.create(imgHSV.size(), CV_8UC1);
	s.create(imgHSV.size(), CV_8UC1);
	Mat out[] = {h,s};	// data is not copied, only the headers...
	int fromTo[] = {0,0, 1,1};	// 0->0 (HSV.H->H), 1->1 (HSV.S->S)
	mixChannels(&imgHSV,1,out,2,fromTo,2); // src, 1 src mtx, dst, 2 dst mtx, mapping, 2 pairs

	// SAT (lowpass)
	satFilter->apply(s,smask);

	// HUE 1 (bandpass), combined with SAT mask
	hue1Filter->apply(h,tmp);
	//inRange(h, Scalar(hue1Mask.low), Scalar(hue1Mask.high), tmp);
	min(tmp,smask,resultHue1Mask);
	
	// HUE 2 (bandpass), combined with SAT mask
	hue2Filter->apply(h,tmp);
	//inRange(h, Scalar(hue2Mask.low), Scalar(hue2Mask.high), tmp);
	min(tmp,smask,resultHue2Mask);
}
