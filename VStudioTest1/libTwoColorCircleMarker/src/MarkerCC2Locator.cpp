#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC2Locator.h"
#include "MarkerCC2.h"

using namespace cv;
using namespace TwoColorCircleMarker;


MarkerCC2Locator::MarkerCC2Locator()
{
	verboseImage=NULL;
}

void MarkerCC2Locator::LocateMarkers(Mat &srcCC, std::list<CvRect> *candidateRectList)
{
	foundValidMarker = false;
	// Markerek leolvasasa
	for (std::list<CvRect>::iterator rectIt = candidateRectList->begin();
		 rectIt != candidateRectList->end();
		 rectIt++)
	{
		MarkerCC2 newMarker;
		newMarker.verboseImage = this->verboseImage;	// For debug/verbose purposes
		newMarker.markerLocator = this;	// Used to validate the read code
		newMarker.readCode(srcCC,*rectIt);

		if (newMarker.isValid)
		{
			foundValidMarker = true;
		}
	}
}

bool MarkerCC2Locator::validateMarkerID(int code)
{
	return true;	// May check a list of valid marker codes here...
}
