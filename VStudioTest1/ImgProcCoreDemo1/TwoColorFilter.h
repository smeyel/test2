#ifndef __TWOCOLORFILTER_H_
#define __TWOCOLORFILTER_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "BandpassFilter.h"
#include "HighpassFilter.h"

using namespace cv;

class TwoColorFilter
{
private:
	Mat tmp,tmpSmask,tmpVmask;

	BandpassFilter internalH1Filter;
	BandpassFilter internalH2Filter;
	HighpassFilter internalSatFilter;
	HighpassFilter internalValFilter;

public:
	BandpassFilter *hue1Filter;
	BandpassFilter *hue2Filter;
	HighpassFilter *satFilter;
	HighpassFilter *valFilter;

	// Constructors
	TwoColorFilter();
	TwoColorFilter(double sTh, double vTh, double h1Low, double h1High, double h2Low, double h2High);

	void apply(Mat &srcBGR, Mat &resultHue1Mask, Mat &resultHue2Mask);

	Mat imgHSV;	// Ha esetleg kell, el lehet érni ezt is...
	Mat smask;	// Saturation mask
	Mat vmask;	// Value mask
	Mat svmask;	// Saturation&Value mask
	Mat hMasked;	// Hue values and 0 where saturation/value are low.
	Mat v;		// Val values of pixels
	Mat s;		// Sat values of pixels
	Mat h;		// Hue values of pixels
};

#endif
