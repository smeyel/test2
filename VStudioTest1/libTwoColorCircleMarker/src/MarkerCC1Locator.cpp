#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC1Locator.h"
#include "MarkerCC1.h"

using namespace cv;
using namespace TwoColorCircleMarker;


MarkerCC1Locator::MarkerCC1Locator()
{
	verboseImage=NULL;
}

void MarkerCC1Locator::LocateMarkers(Mat &srcCC, std::list<CvRect> *candidateRectList)
{
	foundValidMarker = false;
	// Markerek leolvasasa
	for (std::list<CvRect>::iterator rectIt = candidateRectList->begin();
		 rectIt != candidateRectList->end();
		 rectIt++)
	{
		MarkerCC1 newMarker;
		newMarker.verboseImage = this->verboseImage;	// For debug/verbose purposes
		newMarker.markerLocator = this;	// Used to validate the read code
		newMarker.readCode(srcCC,*rectIt);

		if (newMarker.isValid)
		{
			foundValidMarker = true;
		}
	}
}

bool MarkerCC1Locator::validateMarkerID(int majorCode, int minorCode)
{
	if (majorCode==5 && minorCode==85) return true;
	return false;
}
