#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "MarkerCC2Tracker.h"

//#include "MarkerCC2.h"
//
//#include "MarkerCC2Tracker.h"
//
#include "TimeMeasurementCodeDefines.h"
#include "FastColorFilter.h"
//
//#include "DetectionResultExporterBase.h"


using namespace cv;
using namespace TwoColorCircleMarker;

// Config manager
bool MarkerCC2Tracker::ConfigManager::readConfiguration(CSimpleIniA *ini)
{
	visualizeColorCodedFrame = ini->GetBoolValue("MarkerCC2Tracker","visualizeColorCodedFrame",false,NULL);
	return true;	// Successful
}

MarkerCC2Tracker::MarkerCC2Tracker()
{
	initialized = false;
	defaultColorCodeFrame = NULL;
	defaultOverlapMask = NULL;
	defaultVisColorCodeFrame = NULL;
	// Init time measurement
	timeMeasurement = new TimeMeasurement();
	TimeMeasurementCodeDefs::setnames(this->timeMeasurement);
	timeMeasurement->init();
}

MarkerCC2Tracker::~MarkerCC2Tracker()
{
	if (defaultColorCodeFrame != NULL)
	{
		if (defaultColorCodeFrame == colorCodeFrame)
		{
			colorCodeFrame = NULL;
		}
		delete defaultColorCodeFrame;
		defaultColorCodeFrame = NULL;
	}
	if (defaultOverlapMask != NULL)
	{
		if (defaultOverlapMask == overlapMask)
		{
			overlapMask = NULL;
		}
		delete defaultOverlapMask;
		defaultOverlapMask = NULL;
	}
	if (defaultVisColorCodeFrame != NULL)
	{
		if (defaultVisColorCodeFrame == visColorCodeFrame)
		{
			visColorCodeFrame = NULL;
		}
		delete defaultVisColorCodeFrame;
		defaultVisColorCodeFrame = NULL;
	}
	delete timeMeasurement;
}


void MarkerCC2Tracker::init(char *configfilename, bool useDefaultInternalFrames, int width, int height)
{
	if (useDefaultInternalFrames)
	{
		defaultColorCodeFrame = new Mat(height, width,CV_8UC1);
		defaultOverlapMask = new Mat(height, width,CV_8UC1);
		defaultVisColorCodeFrame = new Mat(height, width,CV_8UC3);

		colorCodeFrame = defaultColorCodeFrame;
		overlapMask = defaultOverlapMask;
		visColorCodeFrame = defaultVisColorCodeFrame;
	}

	// Assert: make sure overlap mask is set!
	CV_Assert(overlapMask != NULL);

	// Init FastColorFilter
	// Setup mask creation (define target Mat and observed color code)
	fastColorFilter.maskColorCode[0]=COLORCODE_RED;
	fastColorFilter.maskColorCode[1]=COLORCODE_BLU;
	fastColorFilter.overlapMask=overlapMask;
	fastColorFilter.backgroundColorCode=COLORCODE_WHT;

	// Init config manager
	configManager.init(configfilename);
	twoColorLocator.init(configfilename);
	markerCC2Locator.init(configfilename);

	initialized = true;
}


void MarkerCC2Tracker::processFrame(Mat &src, int cameraID, float timestamp)
{
	// Asserts: are all verbose frames set?
	CV_Assert(initialized == true);
	CV_Assert(colorCodeFrame != NULL);

	timeMeasurement->start(TimeMeasurementCodeDefs::ProcessAll);

	// Apply color filtering. Create masks and color coded image
	timeMeasurement->start(TimeMeasurementCodeDefs::FastColorFilter);
	fastColorFilter.FindMarkerCandidates(src,*colorCodeFrame);
	timeMeasurement->finish(TimeMeasurementCodeDefs::FastColorFilter);

	// Not included into FastColorFilter execution time
	if (configManager.visualizeColorCodedFrame)
	{
		CV_Assert(visColorCodeFrame != NULL);
		timeMeasurement->start(TimeMeasurementCodeDefs::VisualizeDecomposedImage);
		fastColorFilter.VisualizeDecomposedImage(*colorCodeFrame,*visColorCodeFrame);
		timeMeasurement->finish(TimeMeasurementCodeDefs::VisualizeDecomposedImage);
	}

	// --- Processing inputFrame -> resultFrame
	//twoColorLocator.verboseImage = &visColorCodeFrame;
	twoColorLocator.verboseImage = &src;
	timeMeasurement->start(TimeMeasurementCodeDefs::TwoColorLocator);
	twoColorLocator.consolidateFastColorFilterRects(fastColorFilter.candidateRects,fastColorFilter.nextFreeCandidateRectIdx,*colorCodeFrame);
	timeMeasurement->finish(TimeMeasurementCodeDefs::TwoColorLocator);

	// MarkerCC2Locator: locateMarkers
	//markerCC2Locator.verboseImage =  &visColorCodeFrame;
	markerCC2Locator.verboseImage =  &src;
	timeMeasurement->start(TimeMeasurementCodeDefs::LocateMarkers);
	markerCC2Locator.LocateMarkers( *colorCodeFrame, &(twoColorLocator.resultRectangles) );
	timeMeasurement->finish(TimeMeasurementCodeDefs::LocateMarkers);

	timeMeasurement->finish(TimeMeasurementCodeDefs::ProcessAll);
}
