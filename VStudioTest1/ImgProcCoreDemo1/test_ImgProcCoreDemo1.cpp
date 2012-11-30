#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include "TwoColorLocator.h"

#include <time.h>

//#include "marker.h"
//#include "markersearch.h"

using namespace cv;
using namespace std;

void do_test3(const string filename);
void do_test4(const string filename);

const char* wndInput = "Video input";
const char* wndOutput = "Processing result";
const char* wndRed = "Red";
const char* wndBlu = "Blu";
const char* wndSat = "Sat";
const char* wndTmp = "Tmp";

typedef struct _ProcessingTimes
{
	clock_t start, captured, resized, processed, shown;
	double elapsed_captured, elapsed_resized, elapsed_processed, elapsed_shown, maxFps;
	double sum_captured, sum_resized, sum_processed, sum_shown, sum_maxFps;
	double avg_captured, avg_resized, avg_processed, avg_shown, avg_maxFps;
	double frameNum;
} ProcessingTimes;

void initProcessingTimes(ProcessingTimes *times)
{
	times->sum_captured = 0;
	times->sum_resized = 0;
	times->sum_processed = 0;
	times->sum_shown = 0;
	times->sum_maxFps = 0;
	times->frameNum = 0;
}

void mergeNewProcessingTimes(ProcessingTimes *times)
{
	if (times->shown ==  times->start)
	{
		return;	// Invalid data... (too small elapsed time...)
	}

	times->elapsed_captured = (float)times->captured - (float)times->start;
	times->elapsed_resized = (float)times->resized - (float)times->captured;
	times->elapsed_processed = (float)times->processed - (float)times->captured;
	times->elapsed_shown = (float)times->shown - (float)times->processed;
	times->maxFps = 1000 / ((float)times->shown - (float)times->start);

	times->sum_captured += times->elapsed_captured;
	times->sum_resized += times->elapsed_resized;
	times->sum_processed += times->elapsed_processed;
	times->sum_shown += times->elapsed_shown;
	times->sum_maxFps += times->maxFps;
	times->frameNum++;

	times->avg_captured = times->sum_captured / times->frameNum;
	times->avg_resized = times->sum_resized / times->frameNum;
	times->avg_processed = times->sum_processed / times->frameNum;
	times->avg_shown = times->sum_shown / times->frameNum;
	times->avg_maxFps = times->sum_maxFps / times->frameNum;
}

int main()
{
	//do_test3("d:\\SMEyeL\\inputmedia\\gyengeMarkerVideo.MP4");
	do_test4("d:\\SMEyeL\\inputmedia\\gyengeMarkerVideo.MP4");
}

/*void do_test3(const string filename) // video feldogozas - marker kereses szinekkel
{
	VideoCapture capture(filename);
	if (!capture.isOpened())
	{
		cout << "Cannot open input file: " << filename << endl;
	}

	Size videoSize = Size( (int)capture.get(CV_CAP_PROP_FRAME_WIDTH),
						   (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Input resolution: " << videoSize.width << "x" << videoSize.height << endl;

	//namedWindow(wndInput, CV_WINDOW_AUTOSIZE);
	namedWindow(wndOutput, CV_WINDOW_AUTOSIZE);
	//namedWindow(wndTmp, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(wndInput, mouse_callback);
	
	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,400);
	//const Size dsize(320,200);

	ProcessingTimes times;
	initProcessingTimes(&times);

//	initHue2CodeLut();

	while(true)
	{
		times.start = clock();
		// Get next frame
		capture >> inputFrame;
		if (inputFrame.empty())
		{
			cout << "End of video" << endl;
			break;
		}
		times.captured = clock();

		// Resizing image
		resize(inputFrame,resizedFrame,dsize);
		times.resized  = clock();

		// Processing inputFrame -> resultFrame
		processFrame(&resizedFrame,&resultFrame);
		times.processed = clock();

		// show input and output frame
		//imshow(wndInput,resizedFrame);
		imshow(wndOutput,resultFrame);
		times.shown = clock();

		// Measure elapsed times, calculate averages until now...
		mergeNewProcessingTimes(&times);

		// Wait between frames or stop at Esc key...
		int totalFrameTime = times.shown - times.start;
		int delay = (1000/25) - totalFrameTime;
		if (delay < 1)
		{
			delay = 1;
		}
		c = cvWaitKey(delay);
		if (c==27) break;
	}

	printf("\n\nAverage times: capt %4.1lf, resize %4.1lf, proc %4.1lf, show %4.1lf, max %4.1lf FPS\n",
		times.avg_captured, times.avg_resized, times.avg_processed, times.avg_shown, times.avg_maxFps);

	cout << "Press any key..." << endl;
	c = cvWaitKey(0);

	return;
} */


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
//	cvSetMouseCallback(wndInput, mouse_callback);
	
	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,400);
	//const Size dsize(320,200);

	ProcessingTimes times;
	initProcessingTimes(&times);

	//initHue2CodeLut();

	TwoColorLocator twoColorLocator;

	while(true)
	{
		times.start = clock();
		// Get next frame
		capture >> inputFrame;
		if (inputFrame.empty())
		{
			cout << "End of video" << endl;
			break;
		}
		times.captured = clock();

		// Resizing image
		resize(inputFrame,resizedFrame,dsize);
		times.resized  = clock();

		// Processing inputFrame -> resultFrame
		twoColorLocator.verbose = true;
		twoColorLocator.apply(resizedFrame);
		//processFrame(&resizedFrame,&resultFrame);
		times.processed = clock();

		// show input and output frame
		//imshow(wndInput,resizedFrame);
		imshow(wndOutput,twoColorLocator.verboseImage);
		times.shown = clock();

		// Measure elapsed times, calculate averages until now...
		mergeNewProcessingTimes(&times);

		// Wait between frames or stop at Esc key...
		int totalFrameTime = times.shown - times.start;
		int delay = (1000/25) - totalFrameTime;
		if (delay < 1)
		{
			delay = 1;
		}
		c = cvWaitKey(delay);
		if (c==27) break;
	}

	printf("\n\nAverage times: capt %4.1lf, resize %4.1lf, proc %4.1lf, show %4.1lf, max %4.1lf FPS\n",
		times.avg_captured, times.avg_resized, times.avg_processed, times.avg_shown, times.avg_maxFps);

	cout << "Press any key..." << endl;
	c = cvWaitKey(0);

	return;
}
