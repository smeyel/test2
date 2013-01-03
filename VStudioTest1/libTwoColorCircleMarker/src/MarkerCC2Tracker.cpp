#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "MarkerCC2Tracker.h"

//#include "MarkerCC2.h"
//
//#include "MarkerCC2Tracker.h"
//
#include "TimeMeasurementCodeDefines.h"
#include "ConfigManager.h"
#include "FastColorFilter.h"
//
//#include "DetectionResultExporterBase.h"


using namespace cv;
using namespace TwoColorCircleMarker;

MarkerCC2Tracker::MarkerCC2Tracker()
{
	initialized = false;
}

void MarkerCC2Tracker::init()
{
	// Assert: make sure overlap mask is set!
	CV_Assert(overlapMask != NULL);

	// Init FastColorFilter
	// Setup mask creation (define target Mat and observed color code)
	fastColorFilter.maskColorCode[0]=COLORCODE_RED;
	fastColorFilter.maskColorCode[1]=COLORCODE_BLU;
	fastColorFilter.overlapMask=overlapMask;
	fastColorFilter.backgroundColorCode=COLORCODE_WHT;

	initialized = true;
}


void MarkerCC2Tracker::processFrame(Mat &src, int cameraID, float timestamp)
{
	// Asserts: are all verbose frames set?
	CV_Assert(initialized == true);
	CV_Assert(colorCodeFrame != NULL);
	CV_Assert(overlapMask != NULL);
	CV_Assert(visColorCodeFrame != NULL);

	// Apply color filtering. Create masks and color coded image
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::FastColorFilter);
	fastColorFilter.FindMarkerCandidates(src,*colorCodeFrame);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::FastColorFilter);

	// Not included into FastColorFilter execution time
	if (ConfigManager::Current()->verboseColorCodedFrame)
	{
		fastColorFilter.VisualizeDecomposedImage(*colorCodeFrame,*visColorCodeFrame);
	}

	// --- Processing inputFrame -> resultFrame
	//twoColorLocator.verboseImage = &visColorCodeFrame;
	twoColorLocator.verboseImage = &src;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::TwoColorLocator);
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ConsolidateRectangles);
	twoColorLocator.consolidateFastColorFilterRects(fastColorFilter.candidateRects,fastColorFilter.nextFreeCandidateRectIdx,*colorCodeFrame);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ConsolidateRectangles);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::TwoColorLocator);

	// MarkerCC2Locator: locateMarkers
	//markerCC2Locator.verboseImage =  &visColorCodeFrame;
	markerCC2Locator.verboseImage =  &src;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::LocateMarkers);
	markerCC2Locator.LocateMarkers( *colorCodeFrame, &(twoColorLocator.resultRectangles) );
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::LocateMarkers);
}
