#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC1Locator.h"
#include "MarkerCC1.h"

using namespace cv;

MarkerCC1Locator::MarkerCC1Locator()
{
	verboseImage=NULL;
}

void MarkerCC1Locator::LocateMarkers(Mat &hueImage, std::list<CvRect> *candidateRectList)
{
	// Markerek leolvasasa
	for (std::list<CvRect>::iterator rectIt = candidateRectList->begin();
		 rectIt != candidateRectList->end();
		 rectIt++)
	{
		MarkerCC1 newMarker;
		newMarker.readCode(hueImage,*rectIt,verboseImage);
	}
}

