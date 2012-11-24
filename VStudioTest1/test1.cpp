#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include <time.h>

#include "marker.h"
#include "markersearch.h"

using namespace cv;
using namespace std;

void ShowMatrix32FC1Content(Mat mtx);
void ShowMatrix8UC1Content(Mat mtx);

void do_test1();
void do_test2();
void do_test3();

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
	//do_test1();
	//do_test2();
	do_test3();
}




void do_test3() // video feldogozas - marker kereses szinekkel
{
	// Source file
	//const string filename = "d:\\SMEyeL\\inputmedia\\MarkerKockaKovetes1.mp4";
	const string filename = "d:\\SMEyeL\\inputmedia\\gyengeMarkerVideo.MP4";
	//const string filename = "d:\\SMEyeL\\inputmedia\\gyengeMarkerVideo2.MP4";

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
/*	namedWindow(wndBlu, CV_WINDOW_AUTOSIZE);
	namedWindow(wndSat, CV_WINDOW_AUTOSIZE); */
	cvSetMouseCallback(wndInput, mouse_callback);
	
	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,400);
	//const Size dsize(320,200);

	ProcessingTimes times;
	initProcessingTimes(&times);

	initHue2CodeLut();

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
}


void do_test2() // video feldogozas
{
	// Source file
	const string filename = "d:\\temp\\testvideo1.mp4";

	VideoCapture capture(filename);
	if (!capture.isOpened())
	{
		cout << "Cannot open input file: " << filename << endl;
	}

	Size videoSize = Size( (int)capture.get(CV_CAP_PROP_FRAME_WIDTH),
						   (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	cout << "Input resolution: " << videoSize.width << "x" << videoSize.height << endl;

	const char* wndInput = "Video input";
	const char* wndOutput = "Processing result";
	namedWindow(wndInput, CV_WINDOW_AUTOSIZE);
	namedWindow(wndOutput, CV_WINDOW_AUTOSIZE);
	
	Mat inputFrame, resultFrame;
	// Temp frames
	Mat resizedFrame;
	Mat cannyFrame, grayFrame, erodedFrame, dilatedFrame;
	int frameNum = 0;
	char c;

	double lowTh = 0.5;
	double highTh = 2;

	// size for resizing
	//const Size dsize(640,400);
	const Size dsize(320,200);

	// Prepare time measurement
	clock_t time_start,time_captured,time_resized,time_processed,time_shown;

	// For avg calculation
	double sum_elapsed_capture = 0;
	double sum_elapsed_resize = 0;
	double sum_elapsed_process = 0;
	double sum_elapsed_show = 0;
	double sum_maxFps = 0;

	while(true)
	{
		time_start = clock();
		capture >> inputFrame;	// Get next frame
		if (inputFrame.empty())
		{
			cout << "End of video" << endl;
			break;
		}
		frameNum++;
		time_captured = clock();

		// Resizing image
		resize(inputFrame,resizedFrame,dsize);
		time_resized  = clock();

		// Processing inputFrame -> resultFrame
		cvtColor(resizedFrame, grayFrame, CV_BGR2GRAY);

		Canny(grayFrame,cannyFrame,lowTh,highTh);

		Mat element = getStructuringElement(MORPH_RECT, Size(5,5), Point(2,2));
		dilate(cannyFrame,dilatedFrame,element);
		erode(dilatedFrame,erodedFrame,element);
		
		// move to result...
		erodedFrame.copyTo(resultFrame);
		time_processed = clock();

		// show input and output frame
		imshow(wndInput,inputFrame);
		imshow(wndOutput,resultFrame);
		time_shown = clock();

		double elapsed_capture = (float)time_captured - (float)time_start;
		double elapsed_resize = (float)time_resized - (float)time_captured;
		double elapsed_process = (float)time_processed - (float)time_captured;
		double elapsed_show = (float)time_shown - (float)time_processed;
		double maxFps = 1000 / ((float)time_shown - (float)time_start);

		sum_elapsed_capture += elapsed_capture;
		sum_elapsed_resize += elapsed_resize;
		sum_elapsed_process += elapsed_process;
		sum_elapsed_show += elapsed_show;
		sum_maxFps += maxFps;

		printf("Elasped times: capt %4.1lf, resize %4.1lf, proc %4.1lf, show %4.1lf, max %4.1lf FPS\n",
			elapsed_capture, elapsed_resize, elapsed_process, elapsed_show, maxFps);

		int totalFrameTime = time_shown - time_start;
		int delay = (1000/25) - totalFrameTime;
		if (delay < 1)
		{
			delay = 1;
		}

		c = cvWaitKey(delay);
		if (c==27) break;
	}

	double avg_elapsed_capture = sum_elapsed_capture / frameNum;
	double avg_elapsed_resize = sum_elapsed_resize / frameNum;
	double avg_elapsed_process = sum_elapsed_process / frameNum;
	double avg_elapsed_show = sum_elapsed_show / frameNum;
	double avg_maxFps = sum_maxFps / frameNum;
	
	printf("\n\nAverage times: capt %4.1lf, resize %4.1lf, proc %4.1lf, show %4.1lf, max %4.1lf FPS\n",
		avg_elapsed_capture, avg_elapsed_resize, avg_elapsed_process, avg_elapsed_show, avg_maxFps);

	cout << "Press any key..." << endl;
	c = cvWaitKey(0);

	return;
}

void do_test1()
{
/*	Mat alapkep = imread("d:\\temp\\alapkep.png", 1);
	Mat minta = imread("d:\\temp\\minta.png", 1);*/
	Mat alapkep = imread("d:\\temp\\alapkep.jpg");
	Mat minta = imread("d:\\temp\\minta.jpg");

	if ((alapkep.data == NULL) || (minta.data == NULL))
	{
		printf("Cannot load file...\n");
	}
	else
	{
		imshow("Minta",minta);

		int resultW = alapkep.cols - minta.cols + 1;
		int resultH = alapkep.rows - minta.rows + 1;
		printf("Eredmeny merete: w%d, h%d\n",resultW,resultH);

		Mat eredmeny(resultW,resultH,CV_32FC1);

		printf("Matching...");
		matchTemplate(alapkep,minta,eredmeny,CV_TM_SQDIFF);
		printf("OK\n");

		printf("Normalizing...");
		normalize(eredmeny,eredmeny,0,1,NORM_MINMAX, -1, Mat());
		printf("OK\n");

		printf("Searching for minimum SqrDiff...");
		double minVal, maxVal;
		Point minLoc,maxLoc,matchLoc;
		minMaxLoc(eredmeny,&minVal,&maxVal,&minLoc,&maxLoc,Mat());
		matchLoc = minLoc;
		printf("OK\n");

		rectangle(eredmeny, matchLoc,
			Point(matchLoc.x+minta.cols,matchLoc.y+minta.rows),
			Scalar::all(0),2,8,0);

		rectangle(alapkep, matchLoc,
			Point(matchLoc.x+minta.cols,matchLoc.y+minta.rows),
			Scalar::all(0),2,8,0);

		imshow("Eredmeny-alapkepen",alapkep);
		imshow("Eredmeny",eredmeny);

		//ShowMatrix32FC1Content(eredmeny);
	}
	waitKey(0);
	return;
}

void ShowMatrix32FC1Content(Mat mtx)
{
	int resultW = mtx.cols;
	int resultH = mtx.rows;
	printf("32FC1 Eredmeny merete: w%d, h%d\n",resultW,resultH);

	for (int r=0; r<resultH; r++)
	{
		for (int c=0; c<resultW; c++)
		{
			printf("c%d, r%d: ",c,r);
			float pointValue = mtx.at<float>(r,c);	// WRN koordinata sorrend!
			printf("%3.2f\n",pointValue);
		}
		printf("\n");
	}
	printf("ShowMatrixContent done.");
}

void ShowMatrix8UC1Content(Mat mtx)
{
	int resultW = mtx.cols;
	int resultH = mtx.rows;
	printf("8UC1 Eredmeny merete: w%d, h%d\n",resultW,resultH);

	for (int r=0; r<resultH; r++)
	{
		for (int c=0; c<resultW; c++)
		{
			printf("c%d, r%d: ",c,r);
			unsigned int pointValue = mtx.at<unsigned int>(r,c);	// WRN koordinata sorrend!
			printf("%u\n",pointValue);
		}
		printf("\n");
	}
	printf("ShowMatrixContent done.");
}