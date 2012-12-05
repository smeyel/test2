#ifndef __FASTCOLORFILTER_H_
#define __FASTCOLORFILTER_H_

//#include <opencv2\opencv.hpp>
//#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

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

public:
	// code remap and its functions
	uchar CodeRemap[27];
	void resetRemap();
	void enhanceRemap()
	{
		for (int i=0; i<27; i++)
		{
			CodeRemap[i]=i;
			// Enhance R
/*			if (getR(i)==1)
			{
				CodeRemap[i] += rUnit;
			}
			// Enhance G
			if (getG(i)==1)
			{
				CodeRemap[i] += gUnit;
			} */
			// Enhance B
			if (getB(i)==1)
			{
				CodeRemap[i] += bUnit;
			}

		}
	}

	// Init functions (automatically resets the codeRemap)
	void init(int lowLimit, int highLimit);
	void init(int lowR, int highR, int lowG, int highG, int lowB, int highB);

	// Image decomposition (filtering)
	void DecomposeImage(cv::Mat &src, cv::Mat &dst);
	// Code image visualization: amplification of colors in BGR space
	void VisualizeDecomposedImage(cv::Mat &src, cv::Mat &dst);
};

#endif
