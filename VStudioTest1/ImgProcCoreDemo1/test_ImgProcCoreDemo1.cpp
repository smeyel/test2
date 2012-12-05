#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include "TwoColorLocator.h"
#include "MarkerCC1Locator.h"

#include "TimeMeasurementCodeDefines.h"
#include "ConfigManager.h"
#include "FastColorFilter.h"

using namespace cv;
using namespace std;

void do_test4(const string filename);
void do_test5(const string filename);

const char* wndInput = "Video input";
const char* wndOutput = "Processing result";
const char* wndRed = "Red";
const char* wndBlu = "Blu";
const char* wndSat = "Sat";
const char* wndTmp = "Tmp";

TwoColorFilter *twoColorFilter;	// Used by mouse handler

void mouse_callback(int eventtype, int x, int y, int flags, void *param)
{
	if (eventtype == CV_EVENT_LBUTTONDOWN)
	{
		int hue = twoColorFilter->h.at<uchar>(y, x);
		int sat = twoColorFilter->s.at<uchar>(y, x);
		int val = twoColorFilter->v.at<uchar>(y, x);
		int hm = twoColorFilter->hMasked.at<uchar>(y, x);
		int sm = twoColorFilter->smask.at<uchar>(y, x);
		int vm = twoColorFilter->vmask.at<uchar>(y, x);

		cout << "Click at " << x << "-" << y <<
			", h=" << hm << ", s=" << sat << ", v=" << val <<
			", hMasked=" << hm << ", smask=" << sm << ", vmask=" << vm << endl;
	}
}

int main()
{
	ConfigManager::Current()->init("../testini.ini");
	//do_test4("d:\\SMEyeL\\inputmedia\\MarkerCC1\\Single2outerGrn.mp4");
	do_test5("d:\\SMEyeL\\inputmedia\\MarkerCC1\\Single2outerGrn.mp4");
}

void do_test5(const string filename) // video feldogozas - marker kereses szinekkel
{
	VideoCapture capture(filename);
	if (!capture.isOpened())
	{
		cout << "Cannot open input file: " << filename << endl;
	}

	Size videoSize = Size( (int)capture.get(CV_CAP_PROP_FRAME_WIDTH),
						   (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Input resolution: " << videoSize.width << "x" << videoSize.height << endl;

	namedWindow(wndOutput, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(wndOutput, mouse_callback);
	
	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,480);

	TimeMeasurement::instance.init();
	TimeMeasurementCodeDefs::setnames();

	FastColorFilter fastColorFilter;
	fastColorFilter.init(100,150);
	//fastColorFilter.init(100,150,100,150,60,110);
	fastColorFilter.enhanceRemap();

	while(true)
	{
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::FrameAll);
		// Get next frame
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::Capture);
		capture >> inputFrame;
		if (inputFrame.empty())
		{
			cout << "End of video" << endl;
			break;
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::Capture);

		// Resizing image
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::Resize);
		resize(inputFrame,resizedFrame,dsize);
		//blur(resizedFrame,resizedFrame,Size(5,5));
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::Resize);

		// Processing inputFrame -> resultFrame
		Mat colorCodeFrame(resizedFrame.rows,resizedFrame.cols,CV_8UC1);
		Mat visColorCodeFrame(resizedFrame.rows,resizedFrame.cols,CV_8UC3);
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::FastColorFilter);
		fastColorFilter.DecomposeImage(resizedFrame,colorCodeFrame);
		fastColorFilter.VisualizeDecomposedImage(colorCodeFrame,visColorCodeFrame);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FastColorFilter);

		// show input and output frame
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ShowImages);
		imshow(wndOutput, resizedFrame);
		imshow("VisualizedColorCodes", visColorCodeFrame);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ShowImages);

		int totalFrameTime = TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FrameAll);

		int delay = (1000/25) - totalFrameTime;
		if (delay < 1)
		{
			delay = 1;
		}
		c = cvWaitKey(delay);
		if (c==27) break;
		if (c=='p')	// Wait until another keypress
		{
			c = cvWaitKey(0);
		}
	}

	TimeMeasurement::instance.showresults();
	cout << "max fps: " << TimeMeasurement::instance.getmaxfps(TimeMeasurementCodeDefs::FrameAll) << endl;
	cout << "Press any key..." << endl;
	c = cvWaitKey(0);

	return;
}


void do_test4(const string filename) // video feldogozas - marker kereses szinekkel
{
	VideoCapture capture(filename);
	if (!capture.isOpened())
	{
		cout << "Cannot open input file: " << filename << endl;
	}

	Size videoSize = Size( (int)capture.get(CV_CAP_PROP_FRAME_WIDTH),
						   (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Input resolution: " << videoSize.width << "x" << videoSize.height << endl;

	namedWindow(wndOutput, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(wndOutput, mouse_callback);
	
	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,480);
	//const Size dsize(320,200);

	TimeMeasurement::instance.init();
	TimeMeasurementCodeDefs::setnames();
	//initHue2CodeLut();

	TwoColorLocator twoColorLocator;
	MarkerCC1Locator markerCC1Locator;

	twoColorFilter = twoColorLocator.twoColorFilter;

	while(true)
	{
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::FrameAll);
		// Get next frame
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::Capture);
		capture >> inputFrame;
		if (inputFrame.empty())
		{
			cout << "End of video" << endl;
			break;
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::Capture);

		// Resizing image
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::Resize);
		resize(inputFrame,resizedFrame,dsize);
		//blur(resizedFrame,resizedFrame,Size(5,5));
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::Resize);

		// Processing inputFrame -> resultFrame
		twoColorLocator.verboseImage = &resizedFrame;
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::TwoColorLocator);
		twoColorLocator.apply(resizedFrame);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::TwoColorLocator);
		markerCC1Locator.verboseImage =  &resizedFrame;
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::LocateMarkers);
		markerCC1Locator.LocateMarkers( twoColorLocator.twoColorFilter->hMasked, twoColorLocator.twoColorFilter->v, &(twoColorLocator.resultRectangles) );
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::LocateMarkers);

		// show input and output frame
		//imshow(wndInput,resizedFrame);
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ShowImages);
		imshow(wndOutput, resizedFrame);

		//imshow("HS",twoColorLocator.twoColorFilter->hMasked);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ShowImages);

		int totalFrameTime = TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FrameAll);

		int delay = (1000/25) - totalFrameTime;
		//int delay = (5000/25) - totalFrameTime;
		if (delay < 1)
		{
			delay = 1;
		}
		c = cvWaitKey(delay);
		if (c==27) break;
		if (c=='p')	// Wait until another keypress
		{
			c = cvWaitKey(0);
		}
	}

	TimeMeasurement::instance.showresults();
	cout << "max fps: " << TimeMeasurement::instance.getmaxfps(TimeMeasurementCodeDefs::FrameAll) << endl;
	cout << "Press any key..." << endl;
	c = cvWaitKey(0);

	return;
}
