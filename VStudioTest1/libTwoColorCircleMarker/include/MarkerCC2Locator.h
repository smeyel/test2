#ifndef __MARKERCC2LOCATOR_H_
#define __MARKERCC2LOCATOR_H_
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>

#include "DetectionResultExporterBase.h"
#include "TimeMeasurement.h"

using namespace cv;

namespace TwoColorCircleMarker
{
	// TODO: should later have an abstract base class.
	/** Uses the candidate location list created by FastColorFilter and improved by TwoColorFilter
			to locate and validate CC2 (Color Circles type 2) markers. After localization and
			validation, it exports the detection results to a DetectionResultExporterBase.

		To use it,
		- call init() and provide the name of the config file
		- set ResultExporter. Results will be exported there.
		- optionally, set verboseImage to see verbose information
		- for every frame, call LocateMarkers
	*/
	class MarkerCC2Locator
	{
	public:
		/** Constructor
		*/
		MarkerCC2Locator();

		/** Descructor
		*/
		~MarkerCC2Locator();

		/** Initialization, uses the given configuration ini file to set up the process.
		*/
		void init(char *configFileName);

		/** Target image for optional verbose functions
			Set it to a BGR color image to use the verbose functions. Otherwise, leave it NULL.
			The verbose functions may be enabled in the configuration file.
		*/
		Mat *verboseImage;

		/** Target to export the marker detection results.
			Set this before calling LocateMarkers to receive the detection results.
		*/
		DetectionResultExporterBase *ResultExporter;

		/** True if at least one valid marker was located in the last processed frame.
			A marker is valid if the ID was successfully read and it is a valid marker ID.
		*/
		bool foundValidMarker;

		/** Call this to locate the markers.
			Locates markers based on candidate list and reads their ID.
			Results are exported to *ResultExporter.
			@param srcCC	Color coded image (type CV_8UC1) provided by FastColorFilter.
			@param candidateRectList The list of possible marker inner (blue) circle locations,
				provided by TwoColorLocator.
		*/
		void LocateMarkers(Mat &srcCC, std::list<Rect> *candidateRectList);

		/** Returns true if given code is valid
			Called by MarkerCC2 to validate its code.
		*/
		bool validateMarkerID(int code);
	};
}

#endif
