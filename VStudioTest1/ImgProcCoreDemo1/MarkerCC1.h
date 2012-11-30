#ifndef __MARKERCC1_H_
#define __MARKERCC1_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
//#include "TwoColorLocator.h"

using namespace cv;

class MarkerCC1
{
	uchar hue2codeLUT[256];	// erteke -1, 0-3 lehet. -1: ervenytelen, egyebkent 2 bit.
public:
	Point2d center;
	unsigned int markerID;		// real and identified code of the marker
	bool isValid;
	int scanDistance;	// length of scan starting from center (manhattan distance)
	unsigned int codeArray[8];	// 8 directions, temporal color codes...

	MarkerCC1()
	{
		initHue2CodeLut();
	}

	void readCode(Mat &img, CvRect &rect, Mat *verboseImage);
private:
	void initHue2CodeLut();
	bool readCodeAlongLine(Mat &img, int dir);
	void MarkerCC1::validateAndConsolidateMarkerCode();
	CvPoint getEndPoint(int x, int y, int distance, int dir);
};

#endif