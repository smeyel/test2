#ifndef __MARKERCC2_H_
#define __MARKERCC2_H_

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "MarkerCC2Locator.h"

using namespace cv;

namespace TwoColorCircleMarker
{
	class MarkerCC2
	{
	public:
		Point2d center;
		int MarkerID;
		bool isValid;
		bool isCenterValid;	// Center is located. May be true if MarkerID is not valid.

		Mat *verboseImage;
		MarkerCC2Locator *markerLocator;

		// Constructor
		MarkerCC2()
		{
			MarkerID=0;
			isValid=false;
			isCenterValid=false;
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
		Point bitLocations[32];	// 4x8 ellipse points
		uchar rawMarkerIDBitCC[32];	// Color code values of the scanned points

		// --- Marker code processing
		void validateAndConsolidateMarkerCode();

	};
}

#endif
