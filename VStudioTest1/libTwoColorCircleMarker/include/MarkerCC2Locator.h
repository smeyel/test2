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
	// CC2 marker: Color Circles 2
	// This class is used to locate markers of class MarkerCC2
	// and export the detection results to a DetectionResultExporterBase.
	// TODO: should later have an abstract base class.
	class MarkerCC2Locator
	{
	public:
		// Constructor
		MarkerCC2Locator();
		// Descructor
		~MarkerCC2Locator();

		void init(char *configFileName);

		// Target image for optional verbose functions
		Mat *verboseImage;
		// Target to export the marker detection results
		DetectionResultExporterBase *ResultExporter;

		// true if at least one valid marker was located in the last processed frame.
		// A marker is valid if the ID was successfully read and it is a valid marker ID.
		bool foundValidMarker;

		// Entry point.
		// Locates markers based on candidate list and reads their ID.
		// Results are exported to *ResultExporter.
		// candidateRectList is the list of possible marker inner (blue) circle locations.
		void LocateMarkers(Mat &srcCC, std::list<Rect> *candidateRectList);

		// returns true if given code is valid
		// Called by the MarkerCC2 to validate its code.
		bool validateMarkerID(int code);

	};
}

#endif
