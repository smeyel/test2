#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "LowpassFilter.h"

using namespace cv;

void LowpassFilter::apply(Mat &src, Mat &resultMask)
{
	cv::threshold(src,resultMask,threshold,255,THRESH_BINARY);
}
