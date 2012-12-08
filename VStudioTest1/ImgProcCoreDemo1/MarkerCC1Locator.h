#ifndef __MARKERCC1LOCATOR_H_
#define __MARKERCC1LOCATOR_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC1Locator.h"
#include "MarkerCC1.h"

using namespace cv;

// CC1 marker: Color Circles 1
class MarkerCC1Locator
{
public:
	Mat *verboseImage;

	// candidateRectList is the list of possible marker inner (blue) circle locations.
	void LocateMarkers(Mat &srcCC, std::list<CvRect> *candidateRectList);

	MarkerCC1Locator();
};

#endif
