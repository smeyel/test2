#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>

#include "MarkerCC2Tracker.h"

#include "TimeMeasurementCodeDefines.h"
#include "ConfigManagerBase.h"
#include "FastColorFilter.h"

#include "DetectionResultExporterBase.h"

using namespace cv;
using namespace std;
using namespace TwoColorCircleMarker;
using namespace MiscTimeAndConfig;

void do_test6_MarkerCC_FastTwoColorFilter(const string filename);

const char* wndInput = "Video input";
const char* wndOutput = "Processing result";
const char* wndOverlap = "Overlap";
const char* wndColorCode = "ColorCode";

Mat *bgrImage;
Mat *colorCodeImage;

class MyConfigManager : public MiscTimeAndConfig::ConfigManagerBase
{
	// This method is called by init of the base class to read the configuration values.
	virtual bool readConfiguration(CSimpleIniA *ini)
	{
		resizeImage = ini->GetBoolValue("Main","resizeImage",false,NULL);
		showInputImage = ini->GetBoolValue("Main","showInputImage",false,NULL);
		verboseColorCodedFrame = ini->GetBoolValue("Main","verboseColorCodedFrame",false,NULL);
		verboseOverlapMask = ini->GetBoolValue("Main","verboseOverlapMask",false,NULL);
		waitFor25Fps = ini->GetBoolValue("Main","waitFor25Fps",false,NULL);
		pauseIfNoValidMarkers = ini->GetBoolValue("Main","pauseIfNoValidMarkers",false,NULL);
		waitKeyPressAtEnd = ini->GetBoolValue("Main","waitKeyPressAtEnd",false,NULL);
		runMultipleIterations = ini->GetBoolValue("Main","runMultipleIterations",false,NULL);
		return true;
	}

public:
	// --- Settings
	bool resizeImage;
	bool pauseIfNoValidMarkers;
	bool verboseOverlapMask;
	bool verboseColorCodedFrame;
	bool showInputImage;
	bool waitFor25Fps;
	bool waitKeyPressAtEnd;
	bool runMultipleIterations;
};


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

MyConfigManager configManager;
//char *configfilename = "../testini.ini";
char configfilename[] = "../speedtest.ini";

int main()
{
	configManager.init(configfilename);

	int n = 1;
	if (configManager.runMultipleIterations)
	{
		n = 10;
	}
	for(int i=0; i<n; i++)
	{
		cout << "Iteration: " << i << endl;
		do_test6_MarkerCC_FastTwoColorFilter("d:\\SMEyeL\\inputmedia\\MarkerCC2\\MarkerCC2_test2.mp4");
	}

	if (configManager.waitKeyPressAtEnd)
	{
		cout << "Press any key..." << endl;
		char c = cvWaitKey(0);
	}
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
	if (configManager.verboseColorCodedFrame)
	{
		namedWindow(wndColorCode, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(wndColorCode, mouse_callback);
	}

	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,480);	// TODO: should always correspond to the real frame size!
	//const Size dsize(320,240);

	// Init time measurement
	MiscTimeAndConfig::TimeMeasurement timeMeasurement;
	timeMeasurement.init();
	TimeMeasurementCodeDefs::setnames(&timeMeasurement);

	// Init result exporter
	ResultExporter resultExporter;
	resultExporter.open("output.txt");
	resultExporter.currentCamID = 0;

	// --- Setup marker tracker
	TwoColorCircleMarker::MarkerCC2Tracker tracker;
	tracker.setResultExporter(&resultExporter);
	tracker.init(configfilename,true,dsize.width,dsize.height);

	// Setup mouse click handler
	bgrImage = &resizedFrame;
	colorCodeImage = tracker.colorCodeFrame;

	timeMeasurement.start(TimeMeasurementCodeDefs::FullExecution);
	bool pauseDueToSettings = false;	// true means some setting wants to pause the processing
	int frameID=-1;
	while(true)
	{
		frameID++;
		resultExporter.currentFrameIdx = frameID;

		pauseDueToSettings = false;
		timeMeasurement.start(TimeMeasurementCodeDefs::FrameAll);
		// Get next frame
		timeMeasurement.start(TimeMeasurementCodeDefs::Capture);
		capture >> inputFrame;
		if (inputFrame.empty())
		{
			cout << "End of video" << endl;
			break;
		}
		timeMeasurement.finish(TimeMeasurementCodeDefs::Capture);

		// Resizing image
		timeMeasurement.start(TimeMeasurementCodeDefs::Resize);
		if (configManager.resizeImage)
		{
			resize(inputFrame,resizedFrame,dsize);
		}
		else
		{
			// TODO: warning, this makes it unnecessary slow...
			resizedFrame = inputFrame;
		}
		timeMeasurement.finish(TimeMeasurementCodeDefs::Resize);

		float timestamp = (float)frameID;
		timeMeasurement.start(TimeMeasurementCodeDefs::Process);
		tracker.processFrame(resizedFrame,0,timestamp);
		timeMeasurement.finish(TimeMeasurementCodeDefs::Process);

		if (!tracker.getFoundValidMarker() && configManager.pauseIfNoValidMarkers)
		{
			// No valid markers found, settings request processing pause (for inspection).
			cout << "PAUSE: no valid markers found on this frame!" << endl;
			pauseDueToSettings = true;
		}

		// show frames
		timeMeasurement.start(TimeMeasurementCodeDefs::ShowImages);
		if (configManager.verboseOverlapMask)
		{
			imshow(wndOverlap, *tracker.overlapMask);
		}

		if (configManager.showInputImage)
		{
			imshow(wndOutput, resizedFrame);
		}
		if (configManager.verboseColorCodedFrame)
		{
			imshow(wndColorCode, *tracker.visColorCodeFrame);
		}
		timeMeasurement.finish(TimeMeasurementCodeDefs::ShowImages);

		int totalFrameTime = timeMeasurement.finish(TimeMeasurementCodeDefs::FrameAll);

		// Time measurement summary and delay, + pause control
		timeMeasurement.start(TimeMeasurementCodeDefs::InterFrameDelay);
		if (configManager.waitFor25Fps)
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
		timeMeasurement.finish(TimeMeasurementCodeDefs::InterFrameDelay);
	}
	timeMeasurement.finish(TimeMeasurementCodeDefs::FullExecution);

	resultExporter.close();

	cout << "--- Main loop time measurement results:" << endl;
	timeMeasurement.showresults();
	cout << "--- Processing time measurement results:" << endl;
	tracker.timeMeasurement->showresults();

	cout << "--- Further details:" << endl;
	cout << "max fps: " << timeMeasurement.getmaxfps(TimeMeasurementCodeDefs::FrameAll) << endl;
	cout << "Number of processed frames: " << frameID << endl;
	return;
}
