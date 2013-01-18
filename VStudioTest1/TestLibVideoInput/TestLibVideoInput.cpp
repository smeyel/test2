//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This file is part of CL-EyeMulticam SDK
//
// C++ CLEyeMulticamTest Sample Application
//
// For updates and file downloads go to: http://codelaboratories.com/research/view/cl-eye-muticamera-sdk
//
// Copyright 2008-2012 (c) Code Laboratories, Inc. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*#include <conio.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h> */


#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write

#include "VideoInputGeneric.h"
#include "VideoInputPs3Eye.h"

using namespace std;
using namespace cv;

// Main program entry point
int main()
{
	//VideoInputGeneric cam1,cam2;
	VideoInputPs3Eye cam1,cam2;
	cam1.init(0);
	cam2.init(1);

	printf("Use the following keys to change camera parameters:\n"
		"\t'1' - select camera 1\n"
		"\t'2' - select camera 2\n"
		"\t'g' - select gain parameter\n"
		"\t'e' - select exposure parameter\n"
		"\t'z' - select zoom parameter\n"
		"\t'r' - select rotation parameter\n"
		"\t'+' - increment selected parameter\n"
		"\t'-' - decrement selected parameter\n");
	// The <ESC> key will exit the program
	Mat image1(480,640,CV_8UC4);
	Mat image2(480,640,CV_8UC4);
	char key;
	while((key = cvWaitKey(25)) != 27)
	{
		cam1.captureFrame(image1);
		cam2.captureFrame(image2);

		imshow("Default video input 1",image1);
		imshow("Default video input 2",image2);
	}
	return 0;
}
