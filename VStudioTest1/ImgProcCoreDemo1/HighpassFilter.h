#ifndef __LOWPASSFILTER_H_
#define __LOWPASSFILTER_H_

#include <opencv2\core\mat.hpp>
using namespace cv;

class HighpassFilter
{
public:
	double threshold;

	void apply(Mat &src, Mat &resultMask);
};

#endif