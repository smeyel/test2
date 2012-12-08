#ifndef __MARKERCC1LOCATOR_H_
#define __MARKERCC1LOCATOR_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

using namespace cv;

// CC1 marker: Color Circles 1
class MarkerCC1Locator
{
public:
	Mat *verboseImage;

	bool foundValidMarker;	// After every frame, may be queried if there was a valid marker.

	// candidateRectList is the list of possible marker inner (blue) circle locations.
	void LocateMarkers(Mat &srcCC, std::list<CvRect> *candidateRectList);
	// returns true if given code is valid
	bool validateMarkerID(int majorCode, int minorCode);

	MarkerCC1Locator();
};

#endif
