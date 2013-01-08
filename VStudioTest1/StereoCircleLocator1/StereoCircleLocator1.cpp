#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include <time.h>

using namespace cv;
using namespace std;

void doit(const string filenameCam1, const string filenameCam2);

const char* wndCam1 = "Video input 1";
const char* wndCam2 = "Video input 2";

int main()
{
	// The HoughCircles param2 parameter (accumulator threshold) is set for these videos, as
	// the small circles need value 20 to be detected.
	doit("../../../inputmedia/MultiViewVideo1/syncCapture2_CAM1.avi", "../../../inputmedia/MultiViewVideo1/syncCapture2_CAM2.avi");
}

void findAndShowCircleOnFrame(Mat &frame)
{
	Mat src_gray;

	// Convert to grayscale and blur to suppress noise
	cvtColor(frame,src_gray,CV_BGR2GRAY);
	GaussianBlur(src_gray,src_gray,Size(9,9),2,2);
	
	// Find circles
	vector<Vec3f> circles;
	HoughCircles(src_gray,circles,CV_HOUGH_GRADIENT,1,src_gray.rows/8,200,20,0,0);

	// Draw the circles
	for(size_t i = 0; i<circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius=cvRound(circles[i][2]);

		if (radius > 15)
		{
			cout << "Circle REJECTED due to oversize." << endl;
			continue;
		}

		// circle center
		circle(frame,center,3,Scalar(0,255,0),-1,8,0);
		// circle outline
		circle(frame,center,radius,Scalar(0,0,255),3,8,0);

		cout << "Circle found, r=" << radius << ", location " << center.x << "-" << center.y << endl;
	}
}

void doit(const string filenameCam1, const string filenameCam2)
{
	// Opening input videos
	VideoCapture captureCam1(filenameCam1);
	if (!captureCam1.isOpened())
	{
		cout << "Cannot open input file: " << filenameCam1 << endl;
	}

	Size videoSizeCam1 = Size( (int)captureCam1.get(CV_CAP_PROP_FRAME_WIDTH),
						   (int)captureCam1.get(CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Input 1 resolution: " << videoSizeCam1.width << "x" << videoSizeCam1.height << endl;

	VideoCapture captureCam2(filenameCam2);
	if (!captureCam2.isOpened())
	{
		cout << "Cannot open input file: " << filenameCam2 << endl;
	}

	Size videoSizeCam2 = Size( (int)captureCam2.get(CV_CAP_PROP_FRAME_WIDTH),
						   (int)captureCam2.get(CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Input 2 resolution: " << videoSizeCam2.width << "x" << videoSizeCam2.height << endl;

	// Opening windows
	namedWindow(wndCam1, CV_WINDOW_AUTOSIZE);
	namedWindow(wndCam2, CV_WINDOW_AUTOSIZE);

//	cvSetMouseCallback(wndCam1, mouse_callback);
	
	Mat inputFrameCam1, inputFrameCam2, resizedCam1, resizedCam2; //, resultFrame;
	char c;

	// size for resizing
	//const Size dsize(640,400);
	const Size dsize(320,200);

	clock_t timeFrameStart, timeFrameFinish; 

	while(true)
	{
		timeFrameStart = clock();

		// Get next frames
		captureCam1 >> inputFrameCam1;
		if (inputFrameCam1.empty())
		{
			cout << "End of video 1" << endl;
			break;
		}
		captureCam1 >> inputFrameCam2;
		if (inputFrameCam2.empty())
		{
			cout << "End of video 2" << endl;
			break;
		}

		// Resizing images
		resize(inputFrameCam1,resizedCam1,dsize);
		resize(inputFrameCam2,resizedCam2,dsize);

		// Processing frames
		//circle(resizedCam2,Point(250,150),20,Scalar(0,0,0),2,8,0);
		//circle(resizedCam2,Point(150,150),25,Scalar(0,0,0),2,8,0);
		//circle(resizedCam2,Point(50,150),30,Scalar(0,0,0),2,8,0);

		findAndShowCircleOnFrame(resizedCam1);
		findAndShowCircleOnFrame(resizedCam2);

		// show input and output frame
		imshow(wndCam1,resizedCam1);
		imshow(wndCam2,resizedCam2);

		timeFrameFinish = clock();

		// Wait between frames or stop at Esc key...
		int totalFrameTime = timeFrameFinish - timeFrameStart;
		int delay = (1000/25) - totalFrameTime;
		if (delay < 1)
		{
			delay = 1;
		}
		c = cvWaitKey(delay);
		if (c==27) break;
	}
	return;
}
