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

MarkerCC2Locator::~MarkerCC2Locator()
{
}

void MarkerCC2Locator::init(char *configFileName)
{
	MarkerCC2::init(configFileName);
}

void MarkerCC2Locator::LocateMarkers(Mat &srcCC, std::list<Rect> *candidateRectList)
{
	foundValidMarker = false;
	// Markerek leolvasasa
	for (std::list<Rect>::iterator rectIt = candidateRectList->begin();
		 rectIt != candidateRectList->end();
		 rectIt++)
	{
		// TODO: warning, this does not re-create the marker instance for every detection!
		MarkerCC2 newMarker;
		newMarker.verboseImage = this->verboseImage;	// For debug/verbose purposes
		newMarker.markerLocator = this;	// Used to validate the read code
		newMarker.readCode(srcCC,*rectIt);

		if (newMarker.isValid)
		{
			foundValidMarker = true;
		}

		// Exports invalid markers (promising candidate rectangles) as well! (But with isCenterValid=false)
		if ((newMarker.isCenterValid || newMarker.isValid) && ResultExporter!=NULL)
		{
			ResultExporter->writeResult(&newMarker);
		}
	}
}

bool MarkerCC2Locator::validateMarkerID(int code)
{
	if (code == 0x70 || code == 0x4C || code == 0x43 || code == 0x2A || code == 0x7F) return true;
	return false;	// May check a list of valid marker codes here...
}
