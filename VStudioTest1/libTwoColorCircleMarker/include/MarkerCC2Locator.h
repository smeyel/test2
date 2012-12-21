#ifndef __MARKERCC2LOCATOR_H_
#define __MARKERCC2LOCATOR_H_
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>

#include "DetectionResultExporterBase.h"

using namespace cv;

namespace TwoColorCircleMarker
{
	// CC1 marker: Color Circles 1
	class MarkerCC2Locator
	{
	public:
		Mat *verboseImage;
		DetectionResultExporterBase *ResultExporter;


		bool foundValidMarker;	// After every frame, may be queried if there was a valid marker.

		// candidateRectList is the list of possible marker inner (blue) circle locations.
		void LocateMarkers(Mat &srcCC, std::list<Rect> *candidateRectList);
		// returns true if given code is valid
		bool validateMarkerID(int code);

		MarkerCC2Locator();
	};
}

#endif
