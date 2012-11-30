#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "BandpassFilter.h"

using namespace cv;

void BandpassFilter::apply(Mat &src, Mat &resultMask)
{
	inRange(src, Scalar(lowThreshold), Scalar(highThreshold), resultMask);
}
