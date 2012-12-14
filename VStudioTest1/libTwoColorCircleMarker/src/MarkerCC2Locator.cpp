#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC2Locator.h"
#include "MarkerCC2.h"

#include "DetectionResultExporterBase.h"

using namespace cv;
using namespace TwoColorCircleMarker;


MarkerCC2Locator::MarkerCC2Locator()
{
	verboseImage = NULL;
	ResultExporter = NULL;
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

		if ((newMarker.isCenterValid || newMarker.isValid) && ResultExporter!=NULL)
		{
			ResultExporter->writeResult(newMarker.MarkerID,newMarker.center.x,newMarker.center.y,newMarker.isCenterValid,newMarker.isValid);
		}
	}
}

bool MarkerCC2Locator::validateMarkerID(int code)
{
	if (code == 76 || code == 42) return true; //  :)
	return false;	// May check a list of valid marker codes here...
}
