#ifndef __MARKERCC1_H_
#define __MARKERCC1_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
//#include "TwoColorLocator.h"

using namespace cv;

class MarkerCC1
{
	//uchar hue2codeLUT[256];	// erteke -1, 0-3 lehet. -1: ervenytelen, egyebkent 2 bit.
public:
	Point2d center;
	int majorMarkerID;		// real and identified code of the marker
	int minorMarkerID;		// real and identified code of the marker
	bool isValid;

	MarkerCC1()
	{
		//initHue2CodeLut();
		majorMarkerID=0;
		minorMarkerID=0;
		isValid=0;
	}

	void readCode(Mat &srcCC, CvRect &rect, Mat *verboseImage);

	//static void configTwoColorFilter(TwoColorFilter *filter);

private:
	int scanDistance;	// length of scan starting from center (manhattan distance)
//	unsigned int codeArray[8];	// 8 directions, temporal color codes...

	//void initHue2CodeLut();
	bool findBordersAlongLine(Mat &srcCC, int dir, Mat *verboseImage);
	void validateAndConsolidateMarkerCode();
	CvPoint getEndPoint(int x, int y, int distance, int dir);

	void fitBorderEllipses(Mat *verboseImage);
	void scanEllipses(Mat &srcCC, Mat *verboseImage);
	Point getEllipsePointInDirection(RotatedRect baseEllipse,float directionAngle,float distanceMultiplier);

	// Temp variables to find ellipses (in 8 directions)
	Point RedInnerBorders[8];
	Point RedOuterBorders[8];
	RotatedRect innerEllipse;
	RotatedRect outerEllipse;

	// Direction with green area (default: -1)
	// Handles multiple such directions, last processed one overwrites previous ones.
	// Dir=0 is north, increasing clockwise.
//	int dirWithGreen;

	uchar rawMarkerIDBitCC[24];	// CC values
};

#endif
