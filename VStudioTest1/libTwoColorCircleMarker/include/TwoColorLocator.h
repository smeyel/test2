#ifndef __TWOCOLORLOCATOR_H_
#define __TWOCOLORLOCATOR_H_

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "FastColorFilter.h"

using namespace cv;

namespace TwoColorCircleMarker
{
	// Uses the results of the FastColorFilter (which has generated a list of candidate marker rectangles)
	// to check for real marker locations. For every candidate rectangle, it starts to scan for required
	// colors in 4 directions starting from the center. If the colors indicate false detection (cannot be a marker),
	// the candidate is omitted. Otherwise, the rectangle size is updated based on the scans in the 4 directions.
	class TwoColorLocator
	{
	public:
		// Constructors
		TwoColorLocator();

		// Entry point
		void consolidateFastColorFilterRects(Rect* candidateRects, int candidateRectNum, Mat &srcCC);

		// Storage of results
		std::list<Rect> resultRectangles;		// final result created by consolidateRects

		// Image for verbose visualization
		Mat *verboseImage;
	private:
		// Updates the size of a rectangle by scanning the image from the center in
		//	4 directions and finding the borders.
		bool updateRectToRealSize(Mat &srcCC, Rect &newRect, Mat *verboseImage);

		// Goes along a line with a given (inner) color and returns the distance of the "border color" pixels. -1 indicates invalid situation.
		int findColorAlongLine(Mat &srcCC, Point startPoint, Point endPoint, uchar innerColorCode, uchar borderColorCode,Mat *verboseImage);
	};
}

#endif
