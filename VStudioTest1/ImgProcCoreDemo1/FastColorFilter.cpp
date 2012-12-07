#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

#include <assert.h>
#include <math.h>

#include "FastColorFilter.h"


using namespace cv;

// ----------------------- Color remap functions

void FastColorFilter::init()
{
	int r,g,b;
	for(unsigned int i=0; i<512; i++)
	{
		RgbLut[i]=COLORCODE_NONE;

		// Get RGB, scale back to 0-255 to simplify the conditions
		r = (i >> 6) << 5;
		g = ((i >> 3) & 0x07) << 5;
		b = (i & 0x07) << 5;

		if (r >= 115 && g >= 115 && b >= 115 && abs(r-g)<=30 && abs(r-b)<=30 && abs(g-b)<=30 )
		{
			RgbLut[i]=COLORCODE_WHT;
		}
		if (r <= 30 &&  g <= 30 && b <= 32)
		{
			RgbLut[i]=COLORCODE_BLK;
		}
		if (r >= 128 &&  g <= 80 && b <= 80)
		{
			RgbLut[i]=COLORCODE_RED;
		}
		if (r <= 65 &&  g >= 96 && b <= 150)
		{
			RgbLut[i]=COLORCODE_GRN;
		}
		if (r <= 64 &&  g <= 60 && b >= 64)
		{
			RgbLut[i]=COLORCODE_BLU;
		}
	}
}

// ----------------------- Image filtering functions

void FastColorFilter::DecomposeImageCreateMasks(cv::Mat &src, cv::Mat &dst)
{
	// Assert for only 8UC3 input images (BGR)
	assert(src.type() == CV_8UC3);
	// Assert for only 8UC1 output images
	assert(dst.type() == CV_8UC1);
	// Assert dst has same size as src
	assert(src.cols == dst.cols);
	assert(src.rows == dst.rows);
	// Assert masks array (check 0th element)
	assert(masks[0]!=NULL);
	// Assert mask image type is CV_8UC1
	assert(masks[0]->type() == CV_8UC1);
	// Assert mask size
	assert(masks[0]->cols == dst.cols);
	assert(masks[0]->rows == dst.rows);
	// Assert masks array (check 1st element)
	assert(masks[1]!=NULL);
	// Assert mask image type is CV_8UC1
	assert(masks[1]->type() == CV_8UC1);
	// Assert mask size
	assert(masks[1]->cols == dst.cols);
	assert(masks[1]->rows == dst.rows);

	uchar colorCode;
	// Go along every pixel and do the following:
	for (int row=0; row<src.rows; row++)
	{
		// Calculate pointer to the beginning of the current row
		const uchar *ptr = (const uchar *)(src.data + row*src.step);
		// Result pointer
		uchar *resultPtr = (uchar *)(dst.data + row*dst.step);
		// Update mask data pointers
		currentMaskDataPtr[0] =  (uchar *)(masks[0]->data + row*masks[0]->step);
		currentMaskDataPtr[1] =  (uchar *)(masks[1]->data + row*masks[1]->step);

		// Go along every BGR colorspace pixel
		for (int col=0; col<src.cols; col++)
		{
			uchar B = *ptr++;
			uchar G = *ptr++;
			uchar R = *ptr++;
			unsigned int idxR = R >> 5;
			unsigned int idxG = G >> 5;
			unsigned int idxB = B >> 5;
			unsigned int idx = (idxR << 6) | (idxG << 3) | idxB;
			colorCode = RgbLut[idx];
			*resultPtr++ = colorCode;

			// Handle masks (2 masks)
			*(currentMaskDataPtr[0]) = (colorCode==maskColorCode[0])?255:0;
			*(currentMaskDataPtr[1]) = (colorCode==maskColorCode[1])?255:0;
			currentMaskDataPtr[0]++;
			currentMaskDataPtr[1]++;
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
			uchar B = *ptr++;
			uchar G = *ptr++;
			uchar R = *ptr++;
			unsigned int idxR = R >> 5;
			unsigned int idxG = G >> 5;
			unsigned int idxB = B >> 5;
			unsigned int idx = (idxR << 6) | (idxG << 3) | idxB;
			colorCode = RgbLut[idx];
			*resultPtr++ = colorCode;
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
			// Generate B channel
			*resultPtr++ = ((*ptr == COLORCODE_BLU) || (*ptr == COLORCODE_WHT)) ? 255 : 0;
			// Generate G channel
			*resultPtr++ = ((*ptr == COLORCODE_GRN) || (*ptr == COLORCODE_WHT)) ? 255 : 0;
			// Generate R channel
			*resultPtr++ = ((*ptr == COLORCODE_RED) || (*ptr == COLORCODE_WHT)) ? 255 : 0;
			ptr++;
		}
	}
}
