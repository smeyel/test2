#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <time.h>

using namespace cv;
using namespace std;

void do_test2(const string filename);

int main()
{
	do_test2("d:\\SMEyeL\\inputmedia\\gyengeMarkerVideo.MP4");
}

void do_test2(const string filename) // video feldogozas
{
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
