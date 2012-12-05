#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

#include <assert.h>

#include "FastColorFilter.h"

using namespace cv;

void FastColorFilter::resetRemap()
{
	for(int i=0; i<27; i++)
	{
		CodeRemap[i]=i;
	}
}

void FastColorFilter::init(int lowLimit, int highLimit)
{
	resetRemap();
	for(int i=0; i<256; i++)
	{
		if (i>=highLimit)
		{
			LutR[i]=2;
			LutG[i]=2;
			LutB[i]=2;
		}
		else if (i>=lowLimit)
		{
			LutR[i]=1;
			LutG[i]=1;
			LutB[i]=1;
		}
		else
		{
			LutR[i]=0;
			LutG[i]=0;
			LutB[i]=0;
		}
	}
}

void FastColorFilter::init(int lowR, int highR, int lowG, int highG, int lowB, int highB)
{
	resetRemap();
	for(int i=0; i<256; i++)
	{
		if (i>=highR)
		{
			LutR[i]=2;
		}
		else if (i>=lowR)
		{
			LutR[i]=1;
		}
		else
		{
			LutR[i]=0;
		}
	}
	for(int i=0; i<256; i++)
	{
		if (i>=highG)
		{
			LutG[i]=2;
		}
		else if (i>=lowG)
		{
			LutG[i]=1;
		}
		else
		{
			LutG[i]=0;
		}
	}
	for(int i=0; i<256; i++)
	{
		if (i>=highB)
		{
			LutB[i]=2;
		}
		else if (i>=lowB)
		{
			LutB[i]=1;
		}
		else
		{
			LutB[i]=0;
		}
	}
}

void FastColorFilter::DecomposeImage(Mat &src, Mat &dst)
{
	// Assert for only 8UC3 input images (BGR)
	assert(src.type() == CV_8UC3);
	// Assert for only 8UC1 output images
	assert(dst.type() == CV_8UC1);
	// Assert dst has same size as src
	assert(src.cols == dst.cols);
	assert(src.rows == dst.rows);

	uchar colorCode;
	// Go along every pixel and do the following:
	for (int row=0; row<src.rows; row++)
	{
		// Calculate pointer to the beginning of the current row
		const uchar *ptr = (const uchar *)(src.data + row*src.step);
		// Result pointer
		uchar *resultPtr = (uchar *)(dst.data + row*dst.step);
		// Go along every BGR colorspace pixel
		for (int col=0; col<src.cols; col++)
		{
			// Process B channel: use LUT and go on
			colorCode = LutB[*ptr++];
			// Process G channel: use LUT and go on
			colorCode += 3*LutG[*ptr++];
			// Process R channel: use LUT and go on
			colorCode += 9*LutR[*ptr++];

			*resultPtr = CodeRemap[colorCode];
			resultPtr++;
		}
	}
}

void FastColorFilter::VisualizeDecomposedImage(cv::Mat &src, cv::Mat &dst)
{
	// Assert for only 8UC1 input images (colorCodes)
	assert(src.type() == CV_8UC1);
	// Assert for only 8UC3 output images (BGR)
	assert(dst.type() == CV_8UC3);
	// Assert dst has same size as src
	assert(src.cols == dst.cols);
	assert(src.rows == dst.rows);

	uchar colorCode;
	// Go along every pixel and do the following:
	for (int row=0; row<src.rows; row++)
	{
		// Calculate pointer to the beginning of the current row
		const uchar *ptr = (const uchar *)(src.data + row*src.step);
		// Result pointer
		uchar *resultPtr = (uchar *)(dst.data + row*dst.step);
		// Go along every colorCode (uchar) colorspace pixel
		for (int col=0; col<src.cols; col++)
		{
			// Generate B channel: colorCode % 3
			*resultPtr++ = ((*ptr) % 3) * 80;
			// Generate G channel: colorCode % 3
			*resultPtr++ = (((*ptr) % 9) / 3) * 80;
			// Generate R channel: colorCode / 9
			*resultPtr++ = ((*ptr) / 9) * 80;
			ptr++;
		}
	}
}
