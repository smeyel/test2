#ifndef __MARKERCC1_H_
#define __MARKERCC1_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "MarkerCC1Locator.h"

using namespace cv;

namespace TwoColorCircleMarker
{
	class MarkerCC1
	{
	public:
		Point2d center;
		int majorMarkerID;		// real and identified code of the marker
		int minorMarkerID;		// real and identified code of the marker
		bool isValid;

		Mat *verboseImage;
		MarkerCC1Locator *markerLocator;

		// Constructor
		MarkerCC1()
		{
			majorMarkerID=0;
			minorMarkerID=0;
			isValid=false;
			verboseImage = NULL;
			markerLocator = NULL;
		}

		// Read the marker code for a given candidate rectangle.
		// It the read is successful, it is really a valid marker.
		void readCode(Mat &srcCC, CvRect &rect);

	private:
		// --- Used by ellpise fitting
		int scanDistance;	// length of scan starting from center (manhattan distance)
		bool findBordersAlongLine(Mat &srcCC, int dir);
		CvPoint getEndPoint(int x, int y, int distance, int dir);
		void fitBorderEllipses();
		// Temp variables
		Point RedInnerBorders[8];
		Point RedOuterBorders[8];
		RotatedRect innerEllipse;
		RotatedRect outerEllipse;

		// --- Reading marker code areas
		void scanEllipses(Mat &srcCC);
		Point getEllipsePointInDirection(RotatedRect baseEllipse,float directionAngle,float distanceMultiplier, Mat &srcCC);
		// For verbose purposes, stores location of the ellipse scan points for every bit
		Point bitLocations[48];	// 4x4 inner and 4x8 outer ellipse points
		uchar rawMarkerIDBitCC[48];	// Color code values of the scanned points

		// --- Marker code processing
		void validateAndConsolidateMarkerCode();

	};
}

#endif
