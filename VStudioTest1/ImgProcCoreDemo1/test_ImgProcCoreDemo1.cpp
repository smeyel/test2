#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include "TwoColorLocator.h"
#include "MarkerCC1Locator.h"

#include <time.h>

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
	//do_test4("d:\\SMEyeL\\inputmedia\\gyengeMarkerVideo.MP4");
	do_test4("d:\\SMEyeL\\inputmedia\\MarkerCC1\\Single2outerGrn.mp4");
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

	ProcessingTimes times;
	initProcessingTimes(&times);

	//initHue2CodeLut();

	TwoColorLocator twoColorLocator;
	MarkerCC1Locator markerCC1Locator;

	twoColorFilter = twoColorLocator.twoColorFilter;

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
		//blur(resizedFrame,resizedFrame,Size(5,5));
		times.resized  = clock();

		// Processing inputFrame -> resultFrame
		twoColorLocator.verboseImage = &resizedFrame;
		twoColorLocator.apply(resizedFrame);
		markerCC1Locator.verboseImage =  &resizedFrame;
		markerCC1Locator.LocateMarkers( twoColorLocator.twoColorFilter->hMasked, twoColorLocator.twoColorFilter->v, &(twoColorLocator.resultRectangles) );

		//processFrame(&resizedFrame,&resultFrame);
		times.processed = clock();

		// show input and output frame
		//imshow(wndInput,resizedFrame);
		imshow(wndOutput, resizedFrame);

		/*// ------------ Visualize hMasked
		Mat hMaskedVisualizationResult;
		Mat tmp;
		vector<Mat> planes;
	    split(resizedFrame, planes);



		Mat All255 = Mat(
			twoColorLocator.twoColorFilter->hMasked.rows,
			twoColorLocator.twoColorFilter->hMasked.cols, CV_8UC1);
		All255.setTo(Scalar(255));
		twoColorLocator.twoColorFilter->hMasked.copyTo(planes[0]);

		All255.copyTo(planes[1]);
		All255.copyTo(planes[2]);
		merge(planes, tmp);
	    cvtColor(tmp, hMaskedVisualizationResult, CV_HSV2BGR);

		imshow("HS",hMaskedVisualizationResult); */
		imshow("HS",twoColorLocator.twoColorFilter->hMasked);

		times.shown = clock();

		// Measure elapsed times, calculate averages until now...
		mergeNewProcessingTimes(&times);

		// Wait between frames or stop at Esc key...
		int totalFrameTime = times.shown - times.start;
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

	printf("\n\nAverage times: capt %4.1lf, resize %4.1lf, proc %4.1lf, show %4.1lf, max %4.1lf FPS\n",
		times.avg_captured, times.avg_resized, times.avg_processed, times.avg_shown, times.avg_maxFps);

	cout << "Press any key..." << endl;
	c = cvWaitKey(0);

	return;
}
