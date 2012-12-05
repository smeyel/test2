#ifndef __FASTCOLORFILTER_H_
#define __FASTCOLORFILTER_H_

//#include <opencv2\opencv.hpp>
//#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

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

public:
	// Pointers for all masks
	(Mat *)masks[2];
	uchar maskColorCode[2];	// Colorcode for the 2 masks they indicate.

	// code remap and its functions
	uchar CodeRemap[27];
	void resetRemap();
	void enhanceRemap();

	// Init functions (automatically resets the codeRemap)
	void init(int lowLimit, int highLimit);
	void init(int lowR, int highR, int lowG, int highG, int lowB, int highB);

	// Image decomposition (filtering)
	void DecomposeImage(Mat &src, Mat &dst);
	void DecomposeImageCreateMasks(Mat &src, Mat &dst);
	// Code image visualization: amplification of colors in BGR space
	void VisualizeDecomposedImage(Mat &src, Mat &dst);
};

#endif
