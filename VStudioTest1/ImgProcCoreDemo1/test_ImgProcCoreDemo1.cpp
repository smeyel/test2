#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>

#include "TwoColorLocator.h"
#include "MarkerCC2Locator.h"
#include "MarkerCC2.h"

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
const char* wndRed = "Red";
const char* wndBlu = "Blu";
const char* wndOverlap = "Overlap";
const char* wndCode = "Sat";
const char* wndTmp = "Tmp";

FastColorFilter *twoColorFilter;	// Used by mouse handler
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
	//do_test6_MarkerCC_FastTwoColorFilter("d:\\SMEyeL\\inputmedia\\MarkerCC1\\Single2outerGrn.mp4");
	//do_test6_MarkerCC_FastTwoColorFilter("d:\\SMEyeL\\inputmedia\\MarkerCC1\\Valosaghu1.mp4");

	//do_test6_MarkerCC_FastTwoColorFilter("d:\\SMEyeL\\inputmedia\\MarkerCC2\\MarkerCC2_test1.mp4");
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
	c = cvWaitKey(0);
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
		namedWindow(wndCode, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(wndCode, mouse_callback);
	}

	Mat inputFrame, resizedFrame, resultFrame;
	char c;

	// size for resizing
	const Size dsize(640,480);	// TODO: should always correspond to the real frame size!
	//const Size dsize(320,240);

	TimeMeasurement::instance.init();
	TimeMeasurementCodeDefs::setnames();
	//initHue2CodeLut();

	// --- Setup color filtering
	FastColorFilter fastColorFilter;
	//fastColorFilter.init();

	// Create images and masks
	Mat colorCodeFrame(dsize.height, dsize.width,CV_8UC1);
	Mat redMask(dsize.height, dsize.width,CV_8UC1);
	Mat blueMask(dsize.height, dsize.width,CV_8UC1);
	Mat overlapMask(dsize.height, dsize.width,CV_8UC1);
	Mat visColorCodeFrame(dsize.height, dsize.width,CV_8UC3);

	// Setup mouse click handler
	bgrImage = &resizedFrame;
	colorCodeImage = &colorCodeFrame;

	// Setup mask creation (define target Mat and observed color code)
	fastColorFilter.masks[0]=&redMask;
	fastColorFilter.maskColorCode[0]=COLORCODE_RED;
	fastColorFilter.masks[1]=&blueMask;
	fastColorFilter.maskColorCode[1]=COLORCODE_BLU;
	fastColorFilter.overlapMask=&overlapMask;
	fastColorFilter.backgroundColorCode=COLORCODE_WHT;

	// Init result exporter
	ResultExporter resultExporter;
	resultExporter.open("output.txt");
	resultExporter.currentCamID = 0;

	// --- Setup marker locator
	TwoColorLocator twoColorLocator;

	MarkerCC2Locator markerCC2Locator;
	markerCC2Locator.ResultExporter = &resultExporter;

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
			//inputFrame.copyTo(resizedFrame);
			resizedFrame = inputFrame;
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::Resize);

		// Apply color filtering. Create masks and color coded image
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::FastColorFilter);
		//fastColorFilter.DecomposeImageCreateMasksWithOverlap(resizedFrame,colorCodeFrame);
		//fastColorFilter.DecomposeImageCreateOverlap_NoBranch(resizedFrame,colorCodeFrame);
		//fastColorFilter.DecomposeImageCreateOverlap(resizedFrame,colorCodeFrame);
		fastColorFilter.FindMarkerCandidates(resizedFrame,colorCodeFrame);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FastColorFilter);

		// Not included into FastColorFilter execution time
		if (ConfigManager::Current()->verboseColorCodedFrame)
		{
			fastColorFilter.VisualizeDecomposedImage(colorCodeFrame,visColorCodeFrame);
		}

		// --- Processing inputFrame -> resultFrame
		// TwoColorLocator: findInitialRects, consolidateRects

		//twoColorLocator.verboseImage = &visColorCodeFrame;
		twoColorLocator.verboseImage = &resizedFrame;
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::TwoColorLocator);
//		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ApplyOnCC);
		//twoColorLocator.findInitialRects(redMask, blueMask);
//		twoColorLocator.findInitialRectsFromOverlapMask(overlapMask);
//		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ApplyOnCC);
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ConsolidateRectangles);
		twoColorLocator.consolidateFastColorFilterRects(fastColorFilter.candidateRects,fastColorFilter.nextFreeCandidateRectIdx,colorCodeFrame);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ConsolidateRectangles);
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::TwoColorLocator);

		// MarkerCC2Locator: locateMarkers
		//markerCC2Locator.verboseImage =  &visColorCodeFrame;
		markerCC2Locator.verboseImage =  &resizedFrame;
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::LocateMarkers);
		markerCC2Locator.LocateMarkers( colorCodeFrame, &(twoColorLocator.resultRectangles) );
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::LocateMarkers);

		if (!markerCC2Locator.foundValidMarker && ConfigManager::Current()->pauseIfNoValidMarkers)
		{
			// No valid markers found, settings request processing pause (for inspection).
			cout << "PAUSE: no valid markers found on this frame!" << endl;
			pauseDueToSettings = true;
		}

		// show frames
		TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ShowImages);
		if (ConfigManager::Current()->verboseOverlapMask)
		{
			imshow(wndOverlap, overlapMask);
		}

		if (ConfigManager::Current()->showInputImage)
		{
			imshow(wndOutput, resizedFrame);
		}
		if (ConfigManager::Current()->verboseColorCodedFrame)
		{
			imshow(wndCode, visColorCodeFrame);
		}
		TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ShowImages);

		int totalFrameTime = TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FrameAll);
		//cout << totalFrameTime << " ";

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

