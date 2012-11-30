#ifndef __TWOCOLORFILTER_H_
#define __TWOCOLORFILTER_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "BandpassFilter.h"
#include "LowpassFilter.h"

using namespace cv;

class TwoColorFilter
{
private:
	Mat h,s,smask,tmp;

	BandpassFilter internalH1Filter;
	BandpassFilter internalH2Filter;
	LowpassFilter internalSatFilter;

public:
	BandpassFilter *hue1Filter;
	BandpassFilter *hue2Filter;
	LowpassFilter *satFilter;

	// Constructors
	TwoColorFilter();
	TwoColorFilter(double sLow, double h1Low, double h1High, double h2Low, double h2High);

	void apply(Mat &srcBGR, Mat &resultHue1Mask, Mat &resultHue2Mask);

	Mat imgHSV;	// Ha esetleg kell, el lehet érni ezt is...
};

#endif
