#ifndef __VIDEOINPUTPS3EYE_H
#define __VIDEOINPUTPS3EYE_H
#include "VideoInput.h"
#include <opencv2/core/core.hpp>
#include "CLEyeMulticam.h"

#include <windows.h>

class VideoInputPs3Eye : public VideoInput
{
	GUID _cameraGUID;
	CLEyeCameraInstance _cam;
	CLEyeCameraColorMode _mode;
	CLEyeCameraResolution _resolution;
	float _fps;

	// Just for asserts
	int w, h;

public:
	void virtual init(int camID);
	bool virtual captureFrame(cv::Mat &frame);
	void virtual release();
};

#endif
