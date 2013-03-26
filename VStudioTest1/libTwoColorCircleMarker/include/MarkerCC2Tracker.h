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

	/** THis class wraps all functions related to MarkerCC2 markers:
		- localization on a frame
		- estimation of position
		- handling validation of marker codes (by containing the list of valid codes)
		- handling multiple video sources (by storing the cameraID)

		In the simplest configuration, use
			- init() method by asking it to use the defaults,
			- setResultExporter() to set an output target, and
			- call processFrame() for every new image.
	*/
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
		/**
			Indicates wether the init() method has been called or not.
		*/
		bool initialized;

		/**	Default image to store the recognized color codes of all pixels.
			The init() method may be asked to set colorCodeFrame to point to this image.
			Instantiation and destruction is handled internally.
		*/
		Mat *defaultColorCodeFrame;

		/** Default image to store the mask of overlappings.
			The init() method may be asked to set overlapMask to point to this image.
			Instantiation and destruction is handled internally.
		*/
		Mat *defaultOverlapMask;

		/** Default image to store the mask of overlappings.
			The init() method may be asked to set overlapMask to point to this image.
			Instantiation and destruction is handled internally.
		*/
		Mat *defaultVisColorCodeFrame;
	public:
		/** The instance of TimeMeasurement (initialized internally) used to measure
			  elapsed times inside this class.
			Initialized by the constructor and valid process values are
			  set by TimeMeasurementCodeDefs::setnames().
			May be used to query the average elapsed times after execution.
		*/
		TimeMeasurement *timeMeasurement;

		/** Pointer to the image used to store the recognized code colors for every pixel
			  of the input image. The init() method may be asked to use the default image,
			  but you may also override them before calling init().
			May be used for verbose functions.
		*/
		Mat *colorCodeFrame;

		/** Pointer to the image used to store the overlap mask created by FastColorFilter.
			The init() method can set it to the default, or you may set it before calling init().
			May be used for verbose functions.
		*/
		Mat *overlapMask;

		/** Pointer to the image used to store the visualized color code frame. In this, proper RGB color
			  values are used to mark the recognized colors. (And not only color codes as in colorCodeFrame.)
			The init() method can set it to the default, or you may set it before calling init().
			May be used for verbose functions.
		*/
		Mat *visColorCodeFrame;

		/** The FastColorFilter instance used by the tracker. It is used to find
			  locations in the input image, where given colors are present near each other.
			May be used for verbose functions, but you do not need to care about it.
		*/
		FastColorFilter fastColorFilter;

		/** The TwoColorLocator instance used by the tracker. It is used to consolidate
			  the locations which were found by the FastColorFilter. Marker candidate rectangles
			  are created after removing overlappings and performing location and size correction steps.
			May be used for verbose functions, but you do not need to care about it.
		*/
		TwoColorLocator twoColorLocator;

		/** The MarkerCC2Locator instance used by the tracker. It is used to evaluate the marker
			  candidate rectangles created by the TwoColorLocator. The marker code is read, validated and
			  exported using a DetectionResultExporterBase.
			May be used for verbose functions, but you do not need to care about it.
		*/
		MarkerCC2Locator markerCC2Locator;

		/** Constructor
		*/
		MarkerCC2Tracker();

		/** Destructor
		*/
		~MarkerCC2Tracker();
		
		/** Wrapper method to set the DetectionResultExporterBase property of the MarkerCC2Locator.
			Use it to define the data export target.
		*/
		void setResultExporter(DetectionResultExporterBase *exporter)
		{
			markerCC2Locator.ResultExporter = exporter;
		}

		/** Returns true if at least one marker with valid code was found in the last image.
		*/
		bool getFoundValidMarker()
		{
			return markerCC2Locator.foundValidMarker;
		}

		/** Initializes the tracker. You can ask it to use the default internal images, if you do not
			  want to specify them (colorCodeFrame, overlapMask, visColorCodeFrame) by yourself. If you
			  want to use your own images, set the corresponding pointers before calling init().
			@param configfilename	Name of the config file.
			@param useDefaultInternalFrames	If true, the pointers will be set to the default values
				after creating the default images.
			@param width	Width of the image if useDefaultInternalFrames is true.
			@param height	Height of the image if useDefaultInternalFrames is true.
		*/
		void init(char *configfilename, bool useDefaultInternalFrames=false, int width=0, int height=0);

		/** Interface for processing a new frame. It contains: color filtering, marker localization
			  (accelerated by location predicion if available) and marker code validation.
			Automatically adds tracking data to results from previous frames.
			@param src	Input image (BGR encoded color image)
			@param cameraID	Identifier of the camera the image came from. It is forwarded to the result export,
				together with the new data.
			@param timestamp Exact time of the shutter of the image. It is forwarded together with the results.
		*/
		void processFrame(Mat &src, int cameraID, float timestamp);
	};

}

#endif
