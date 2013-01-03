#ifndef __MARKERCC2TRACKER_G_
#define __MARKERCC2TRACKER_G_

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>

#include "TwoColorLocator.h"
#include "MarkerCC2Locator.h"

#include "TimeMeasurement.h"

#include "DetectionResultExporterBase.h"

#include "ConfigManagerBase.h"

using namespace cv;
using namespace MiscTimeAndConfig;

namespace TwoColorCircleMarker
{

	// THis class wraps all functions related to MarkerCC2 markers:
	// - localization on a frame
	// - estimation of position
	// - handling validation of marker codes
	// - handling multiple video sources
	class MarkerCC2Tracker
	{
		// Internal configuration class
		class ConfigManager : public MiscTimeAndConfig::ConfigManagerBase
		{
			// This method is called by init of the base class to read the configuration values.
			virtual bool readConfiguration(CSimpleIniA *ini);
		public:
			// Show verbose frames
			bool visualizeColorCodedFrame;
		};
		ConfigManager configManager;

	private:
		bool initialized;

		// Default internal frames
		Mat *defaultColorCodeFrame;
		Mat *defaultOverlapMask;
		Mat *defaultVisColorCodeFrame;
	public:
		TimeMeasurement *timeMeasurement;

		// Pointers to frames for verbose functions
		Mat *colorCodeFrame;
		Mat *overlapMask;
		Mat *visColorCodeFrame;

		// Submodules
		FastColorFilter fastColorFilter;
		TwoColorLocator twoColorLocator;
		MarkerCC2Locator markerCC2Locator;

		// Constructor
		MarkerCC2Tracker();
		// Destructor
		MarkerCC2Tracker::~MarkerCC2Tracker();
		// Setters, getters
		void setResultExporter(DetectionResultExporterBase *exporter)
		{
			markerCC2Locator.ResultExporter = exporter;
		}
		bool getFoundValidMarker()
		{
			return markerCC2Locator.foundValidMarker;
		}

		// Init
		void init(char *configfilename, bool useDefaultInternalFrames=false, int width=0, int height=0);

		// Interface for processing a new frame
		// Contains: color filtering, marker localization
		//	(accelerated by location predicion if available)
		// Automatically adds tracking data to results from previous frames
		void processFrame(Mat &src, int cameraID, float timestamp);
	};

}

#endif
