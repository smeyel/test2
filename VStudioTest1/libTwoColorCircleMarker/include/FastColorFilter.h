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

#define MAXDETECTIONRECTNUM 1000
#define MAXCANDIDATERECTNUM 1000

using namespace cv;

namespace TwoColorCircleMarker
{
	/** Represents a detection of marker candidate. Used to keep track of the blobs to
		finally get the location and size of the blob.
	*/
	struct DetectionRect
	{
		int rowMin;	// First row of detection, set at first encounter
		//int rowMax;	// Last row, set at the end of detection
		int colMin, colMax;	// horizontal boundaries, kept up-to-date
		bool isAlive;	// is this struct used at all?
		bool isDetectedInCurrentRow;	// Was there a detection in the current row?
	};

	/** Searches for locations in an image, where there are two given colors beside each other,
			and around them, there is the background color. Input is a BGR image, the output is
			a 0-255 binary mask and a list of marker candidate location rectangles.

		To use,
		- set maskColorCode[], backgroundColorCode and overlapMask
		- call FindMarkerCandidates()
		- access results in candidateRects[] using nextFreeCandidateRectIdx.
	*/
	class FastColorFilter
	{
/*		uchar LutR[256];
		uchar LutG[256];
		uchar LutB[256]; */

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

		/** Sets LUT to given colorcode for given RGB color.
			Used for runtime color LUT adjustments.
		*/
		void setLutItem(uchar r, uchar g, uchar b, uchar colorCode)
		{
			unsigned int idxR = r >> 5;
			unsigned int idxG = g >> 5;
			unsigned int idxB = b >> 5;
			unsigned int idx = (idxR << 6) | (idxG << 3) | idxB;
			RgbLut[idx] = colorCode;
		}

		/**	The two color codes searched for beside each other by the color filter. The second has to appear
				right after the first one to generate a marker location candidate.
			Set these before using the filter.
		*/
		uchar maskColorCode[2];

		/**	The color code of the background. Before the color defined by maskColorCode[0], this color has to
				be present in the image, as that color is supposed to be found around the circles of the marker.
			Set this before using the filter.
		*/
		uchar backgroundColorCode;

		/** Overlap mask: the image containing the locations of color co-occurrance (value 255).
			This is the result of the color search, and it is used by 
			Set this to point to a CV_8UC1 image with the same size as the input images before using the filter.
		*/
		Mat *overlapMask;

		/** The result of the filter. It contains the marker location candidate rectangles.
			nextFreeCandidateRectIdx is the index of the first unused array element.
		*/
		Rect candidateRects[MAXCANDIDATERECTNUM];
		/** The index of the first unused array element of candidateRects.
		*/
		int nextFreeCandidateRectIdx;

		// --- Image decomposition functions ("entry points")
		/** This method starts the filtering of a frame. It identifies the colors and
				locates the marker candidate areas.
			Call this to use the filter. The result is placed into candidateRects.
			@param src	Input BGR image.
			@param dst	Output image with color codes.
		*/
		void FindMarkerCandidates(cv::Mat &src, cv::Mat &dst);

		/** This method only recognizes the colors in the image, but does not locate marker candidates.
			Only for special applications.
			@param src	Input BGR image.
			@param dst	Output image with color codes.
		*/
		void DecomposeImage(Mat &src, Mat &dst);

		// --- Helpers
		/** This method may be used to visualize the decomposed image. It assigns user interpretable colors
				to the recognized color codes.
			@param src	Input image containing the color codes.
			@param dst	Output BGR image.
		*/
		void VisualizeDecomposedImage(Mat &src, Mat &dst);
	};
}

#endif
