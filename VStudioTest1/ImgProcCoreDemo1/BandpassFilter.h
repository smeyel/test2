#ifndef __BANDPASSFILTER_H_
#define __BANDPASSFILTER_H_

#include <opencv2\core\mat.hpp>
using namespace cv;

class BandpassFilter
{
public:
	double highThreshold;
	double lowThreshold;

	void apply(Mat &src, Mat &resultMask);
};

#endif
