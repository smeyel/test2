#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>

//#include "TwoColorLocator.h"
//#include "MarkerCC2Locator.h"
//#include "MarkerCC2.h"

#include "MarkerCC2Tracker.h"

#include "TimeMeasurementCodeDefines.h"
#include "ConfigManager.h"
#include "FastColorFilter.h"

#include "DetectionResultExporterBase.h"

#define WAITKEYPRESSATEND	// Wait for keypress at the end (disable for profiling!)
//#define MULTIPLEITERATIONS	// Perform everything multiple times (enable for profiling)

using namespace cv;
using namespace std;
using namespace TwoColorCircleMarker;
using namespace MiscTimeAndConfig;

void do_test4(const string filename);
void do_test5(const string filename);
void do_test6_MarkerCC_FastTwoColorFilter(const string filename);

const char* wndInput = "Video input";
const char* wndOutput = "Processing result";
const char* wndOverlap = "Overlap";
const char* wndColorCode = "ColorCode";

Mat *bgrImage;
Mat *colorCodeImage;

void mouse_callback(int eventtype, int x, int y, int flags, void *param)
{
	if (eventtype == CV_EVENT_LBUTTONDOWN)
	{
		if (bgrImage && colorCodeImage)
		{
			Vec3b result = bgrImage->at<Vec3b>(y,x);
			cout << "Click at " << x << "-" << y <<
				" R=" << (int)(result[2] >> 5 << 5) <<
				" G=" << (int)(result[1] >> 5 << 5) <<
				" B=" << (int)(result[0] >> 5 << 5) <<
				" ColorCode=" << (int)(colorCodeImage->at<uchar>(y,x)) << endl;
		}
		else
		{
			cout << "Click. Error: brgImage and colorCodeImage pointers not set." << endl;
		}
	}
}

class ResultExporter : public TwoColorCircleMarker::DetectionResultExporterBase
{
	ofstream stream;
public:
	void open(char *filename)
	{
		stream.open(filename);
	}

	void close()
	{
		stream.flush();
		stream.close();
	}

	int currentFrameIdx;
	int currentCamID;

	virtual void writeResult(MarkerBase *marker)
	{
		stream << "FID:" << currentFrameIdx << ",CID:" << currentCamID << " ";
		marker->exportToTextStream(&stream);
		stream << endl;
	}
};

int main()
{
	TwoColorCircleMarker::ConfigManager::Current()->init("../testini.ini");
	//TwoColorCircleMarker::ConfigManager::Current()->init("../speedtest.ini");
#ifdef MULTIPLEITERATIONS
	for(int i=0; i<10; i++)
	{
		cout << "Iteration: " << i << endl;
#endif
		do_test6_MarkerCC_FastTwoColorFilter("d:\\SMEyeL\\inputmedia\\MarkerCC2\\MarkerCC2_test2.mp4");
#ifdef MULTIPLEITERATIONS
	}
#endif

#ifdef WAITKEYPRESSATEND
	cout << "Press any key..." << endl;
	char c = cvWaitKey(0);
#endif
}

void do_test6_MarkerCC_FastTwoColorFilter(const string filename) // video feldogozas - marker kereses szinekkel
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
	if (ConfigManager::Current()->verboseColorCodedFrame)
	{
		namedWindow(wndColorCode, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(wndColorCode, mouse_callback);
	}

	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,480);	// TODO: should always correspond to the real frame size!
	//const Size dsize(320,240);

	TimeMeasurement::instance.init();
	TimeMeasurementCodeDefs::setnames();

	// Init result exporter
	ResultExporter resultExporter;
	resultExporter.open("output.txt");
	resultExporter.currentCamID = 0;

	// --- Setup marker tracker
	TwoColorCircleMarker::MarkerCC2Tracker tracker;
	tracker.setResultExporter(&resultExporter);
	tracker.init(true,dsize.width,dsize.height);

	// Setup mouse click handler
	bgrImage = &resizedFrame;
	colorCodeImage = tracker.colorCodeFrame;

	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::FullExecution);
	bool pauseDueToSettings = false;	// true means some setting wants to pause the processing
	int frameID=-1;
	while(true)
	{
		frameID++;
		resultExporter.currentFrameIdx = frameID;

		pauseDueToSettings = false;
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
		if (ConfigManager::Current()->resizeImage)
		{
			resize(inputFrame,resizedFrame,dsize);
		}
		else
		{
			// TODO: warning, this makes it unnecessary slow...
			resizedFrame = inputFrame;
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::Resize);

		float timestamp = (float)frameID;
		tracker.processFrame(resizedFrame,0,timestamp);

		if (!tracker.getFoundValidMarker() && ConfigManager::Current()->pauseIfNoValidMarkers)
		{
			// No valid markers found, settings request processing pause (for inspection).
			cout << "PAUSE: no valid markers found on this frame!" << endl;
			pauseDueToSettings = true;
		}

		// show frames
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ShowImages);
		if (ConfigManager::Current()->verboseOverlapMask)
		{
			imshow(wndOverlap, *tracker.overlapMask);
		}

		if (ConfigManager::Current()->showInputImage)
		{
			imshow(wndOutput, resizedFrame);
		}
		if (ConfigManager::Current()->verboseColorCodedFrame)
		{
			imshow(wndColorCode, *tracker.visColorCodeFrame);
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ShowImages);

		int totalFrameTime = TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FrameAll);

		// Time measurement summary and delay, + pause control
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::InterFrameDelay);
		if (ConfigManager::Current()->waitFor25Fps)
		{
			int delay = (1000/25) - totalFrameTime;
			if (delay < 1)
			{
				delay = 1;
			}
			c = cvWaitKey(delay);
			if (c==27) break;
			if (c=='p' || pauseDueToSettings)	// Wait until another keypress
			{
				c = cvWaitKey(0);
			}
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::InterFrameDelay);
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FullExecution);

	resultExporter.close();

	TimeMeasurement::instance.showresults();
	cout << "max fps: " << TimeMeasurement::instance.getmaxfps(TimeMeasurementCodeDefs::FrameAll) << endl;
	cout << "Number of processed frames: " << frameID << endl;
	return;
}
