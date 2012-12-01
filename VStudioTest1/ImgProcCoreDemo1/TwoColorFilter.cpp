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
	valFilter = &internalValFilter;
}

// Prepares default filters (pointers may be redirected...)
TwoColorFilter::TwoColorFilter(double sTh, double vTh, double h1Low, double h1High, double h2Low, double h2High)
{
	internalH1Filter.lowThreshold = h1Low;
	internalH1Filter.highThreshold = h1High;
	internalH2Filter.lowThreshold = h2Low;
	internalH2Filter.highThreshold = h2High;
	internalSatFilter.threshold = sTh;
	internalValFilter.threshold = vTh;

	hue1Filter = &internalH1Filter;
	hue2Filter = &internalH2Filter;
	satFilter = &internalSatFilter;
	valFilter = &internalValFilter;
}

void TwoColorFilter::apply(Mat &srcBGR, Mat &resultHue1Mask, Mat &resultHue2Mask)
{
	// Extract hue channel
	cvtColor(srcBGR, imgHSV, CV_BGR2HSV);
	h.create(imgHSV.size(), CV_8UC1);
	s.create(imgHSV.size(), CV_8UC1);
	v.create(imgHSV.size(), CV_8UC1);
	Mat out[] = {h,s,v};	// data is not copied, only the headers...
	int fromTo[] = {0,0, 1,1, 2,2};	// 0->0 (HSV.H->H), 1->1 (HSV.S->S), 2->2 (HSV.V->V)
	mixChannels(&imgHSV,1,out,3,fromTo,3); // src, 1 src mtx, dst, 3 dst mtx, mapping 3 pairs

	// SAT (highpass)
	satFilter->apply(s,smask);
	// VAL (highpass)
	valFilter->apply(v,vmask);

	min(smask,vmask,svmask);	// Combined S-V (binary) mask

	// For other purposes, create a masked hue image
	min(h,svmask,hMasked);	// Used later, like at marker color recognition

	// HUE 1 (bandpass), combined with SAT mask
	hue1Filter->apply(h,tmp);
	//inRange(h, Scalar(hue1Mask.low), Scalar(hue1Mask.high), tmp);
	min(svmask,tmp,resultHue1Mask);

	// HUE 2 (bandpass), combined with SAT mask
	hue2Filter->apply(h,tmp);
	//inRange(h, Scalar(hue2Mask.low), Scalar(hue2Mask.high), tmp);
	min(svmask,tmp,resultHue2Mask);
}
