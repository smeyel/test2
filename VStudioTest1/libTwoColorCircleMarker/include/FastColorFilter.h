#ifndef __FASTCOLORFILTER_H_
#define __FASTCOLORFILTER_H_

#include <opencv2/core/mat.hpp>

#define COLORCODE_INITIAL 254	// A color code different from all other ones. Used as initial (unknown) value.
#define COLORCODE_NONE 0
#define COLORCODE_RED 1
#define COLORCODE_GRN 2
#define COLORCODE_BLU 3
#define COLORCODE_WHT 4
#define COLORCODE_BLK 5

#define MAXDETECTIONRECTNUM 100
#define MAXCANDIDATERECTNUM 100

using namespace cv;

namespace TwoColorCircleMarker
{
	// Represents a detection of marker candidate. Used to keep track of the blobs to
	//	finally get the location and size of the blob.
	struct DetectionRect
	{
		int rowMin;	// First row of detection, set at first encounter
		//int rowMax;	// Last row, set at the end of detection
		int colMin, colMax;	// horizontal boundaries, kept up-to-date
		bool isAlive;	// is this struct used at all?
		bool isDetectedInCurrentRow;	// Was there a detection in the current row?
	};

	// Warning: do not re-create for every frame! Try to re-use!
	class FastColorFilter
	{
		uchar LutR[256];
		uchar LutG[256];
		uchar LutB[256];

		// Used only by init() internally
		static const uchar rUnit = 9;
		static const uchar gUnit = 3;
		static const uchar bUnit = 1;

		uchar getR(uchar code)
		{
			return code/9;
		}
		uchar getG(uchar code)
		{
			return (code % 9) / 3;
		}
		uchar getB(uchar code)
		{
			return code % 3;
		}

		// --- Color remapping
		// code remap and its functions
		uchar RgbLut[512];	// 3 bit / RGB color layer -> 9 bit, 512 values. 0 means undefined color.

		// Used by FindMarkerCandidates
		struct DetectionRect detectionRects[MAXDETECTIONRECTNUM];
		int lastUsedDetectionRectIdx;
		void StartNewFrame();
		void RegisterDetection(int row, int colStart, int colEnd);
		void FinishRow(int rowIdx);

		// --- inits RgbLut, called from constructor
		void init();

	public:
		// Constructor
		FastColorFilter()
		{
			init();
		}

		// --- Mask colors, needs to be set according to marker colors.
		uchar maskColorCode[2];	// Colorcode for the 2 masks they indicate.
		uchar backgroundColorCode;	// Colorcode for the color outside the marker circles

		// --- Results
		// Overlap mask
		Mat *overlapMask;
		// List of marker candidate rectangles
		Rect candidateRects[MAXCANDIDATERECTNUM];
		int nextFreeCandidateRectIdx;

		// --- Image decomposition functions ("entry points")
		// Color recognition and marker candidate localization
		void FindMarkerCandidates(cv::Mat &src, cv::Mat &dst);
		// Color recognition only (unlikely to be used)
		void DecomposeImage(Mat &src, Mat &dst);

		// --- Helpers
		// Code image visualization: amplification of colors in BGR space
		void VisualizeDecomposedImage(Mat &src, Mat &dst);
	};
}

#endif
