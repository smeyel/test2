#ifndef __VIDEOINPUTGENERIC_H
#define __VIDEOINPUTGENERIC_H
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#include "VideoInput.h"

class VideoInputGeneric : public VideoInput
{
	cv::VideoCapture *capture;
	
public:
	void virtual init(int camID);
	void virtual init(const char *filename);
	bool virtual captureFrame(cv::Mat &frame);
	void virtual release();
};

#endif
