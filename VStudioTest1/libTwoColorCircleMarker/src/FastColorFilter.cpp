#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>

#include <assert.h>
#include <math.h>

#include "FastColorFilter.h"

// REG colors for "unknown color"
#define NONE_R	100
#define NONE_G	0
#define NONE_B	100

#define OVERLAPCHECKLENGTH 5	// Max distance of two colors to trigger overlap

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

		if (r >= 115 && g >= 115 && b >= 115 && abs(r-g)<=35 && abs(r-b)<=35 && abs(g-b)<=35 )
		{
			RgbLut[i]=COLORCODE_WHT;
		}
		if (r <= 64 &&  g <= 64 && b <= 64 && abs(r-g)<=32 && abs(r-b)<=32 && abs(g-b)<=32)
		{
			RgbLut[i]=COLORCODE_BLK;
		}
		if (r >= 128 &&  r >= g+64 && r >= b+32 || (r < 128 && r >= g+64 && r >= b+64))
		{
			RgbLut[i]=COLORCODE_RED;
		}
		if (r <= 65 &&  g >= 96 && b <= 150 || (g < 96 && g >= 64 && g >= r+64 && g >= r+64))
		{
			RgbLut[i]=COLORCODE_GRN;
		}
		if (r <= 64 &&  g <= 64 && b >= 64)
		{
			RgbLut[i]=COLORCODE_BLU;
		}
	}
}

// ----------------------- Image filtering functions

// Also creates overlap mask
void FastColorFilter::DecomposeImageCreateMasksWithOverlap(cv::Mat &src, cv::Mat &dst)
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

	// Assert masks array (check 1st element)
	assert(overlapMask!=NULL);
	// Assert mask image type is CV_8UC1
	assert(overlapMask->type() == CV_8UC1);
	// Assert mask size
	assert(overlapMask->cols == dst.cols);
	assert(overlapMask->rows == dst.rows);

	uchar colorCode;

	// Mask 0 shift counter:
	//	Set when mask[0] is active. Otherwise decreased until 0.
	//	Used to find areas where mask[1] is active and mask[0] was
	//	also active not so far away.
	//	Processed in every row separately.
	int mask0shiftCounter;
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
		currentOverlapMaskDataPtr = (uchar *)(overlapMask->data + row*overlapMask->step);

		mask0shiftCounter = 0;

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
			*currentOverlapMaskDataPtr = 0;

			// Process mask overlap
			if (*(currentMaskDataPtr[0]) == 255)
			{
				// Mask 0 active, trigger checking
				mask0shiftCounter = OVERLAPCHECKLENGTH;
			}
			else
			{
				if (mask0shiftCounter>0)
				{
					// Still checking
					if (*(currentMaskDataPtr[1]) == 255)
					{
						// Overlap
						*currentOverlapMaskDataPtr = 255;
					}
					else
					{
						// We do not decrease this during overlap
						// This helps detection of full overlap and not
						//	only its border...
						mask0shiftCounter--;
					}
				}
			}

			currentMaskDataPtr[0]++;
			currentMaskDataPtr[1]++;
			currentOverlapMaskDataPtr++;
		}
	}
}


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
			*resultPtr++ = ((*ptr == COLORCODE_BLU) || (*ptr == COLORCODE_WHT)) ? 255 : (*ptr == COLORCODE_NONE ? NONE_B : 0);
			// Generate G channel
			*resultPtr++ = ((*ptr == COLORCODE_GRN) || (*ptr == COLORCODE_WHT)) ? 255 : (*ptr == COLORCODE_NONE ? NONE_G : 0);
			// Generate R channel
			*resultPtr++ = ((*ptr == COLORCODE_RED) || (*ptr == COLORCODE_WHT)) ? 255 : (*ptr == COLORCODE_NONE ? NONE_R : 0);
			ptr++;
		}
	}
}
