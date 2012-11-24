#ifndef _MARKER_H_
#define _MARKER_H_

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

using namespace cv;
using namespace std;

typedef struct _Marker {
	Point2d center;
	unsigned int markerID;		// real and identified code of the marker
	bool isValid;
	int scanDistance;	// length of scan starting from center (manhattan distance)
	unsigned int codeArray[8];	// 8 directions, temporal color codes...
} Marker;

void initHue2CodeLut();
//void initMarker(Marker& marker, int x, int y);
//bool readCodeAlongLine(Mat &img, Marker &marker, int dir);
//void validateAndConsolidateMarkerCode(Marker &marker);
void readMarkerCode(Mat &img, CvRect &rect, Marker &marker, Mat *debugImage);

#endif
