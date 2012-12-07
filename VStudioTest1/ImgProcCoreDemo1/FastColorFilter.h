#ifndef __FASTCOLORFILTER_H_
#define __FASTCOLORFILTER_H_

//#include <opencv2\opencv.hpp>
//#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

#define COLORCODE_NONE 0
#define COLORCODE_RED 1
#define COLORCODE_GRN 2
#define COLORCODE_BLU 3
#define COLORCODE_WHT 4
#define COLORCODE_BLK 5


using namespace cv;

// Warning: do not re-create for every frame! Try to re-use!
class FastColorFilter
{
	uchar LutR[256];
	uchar LutG[256];
	uchar LutB[256];

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

	// Internal use by filtering
	// Current data pointers for all masks, indexed by colorcode
	(uchar*)currentMaskDataPtr[2];

	// --- Color remapping
	// code remap and its functions
	uchar RgbLut[512];	// 3 bit / RGB color layer -> 9 bit, 512 values. 0 means undefined color.

public:
	// --- Mask management
	// Pointers for all masks
	(Mat *)masks[2];
	// Color code for the masks to indicate
	uchar maskColorCode[2];	// Colorcode for the 2 masks they indicate.

	// --- inits RgbLut
	void init();

	// --- Image decomposition (filtering)
	void DecomposeImage(Mat &src, Mat &dst);
	void DecomposeImageCreateMasks(Mat &src, Mat &dst);
	// Code image visualization: amplification of colors in BGR space
	void VisualizeDecomposedImage(Mat &src, Mat &dst);
};

#endif
