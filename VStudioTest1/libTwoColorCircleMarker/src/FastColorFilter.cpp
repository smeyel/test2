#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>

#include <assert.h>

#include "FastColorFilter.h"

#include "Logger.h"

using namespace std;
using namespace Logging;

#define LOG_TAG "SMEyeL::FastColorFilter"

// REG colors for "unknown color"
#define NONE_R	100
#define NONE_G	0
#define NONE_B	100

// Max. how many pixels may come after mask color 0 and before mask color 1 to accept marker candidate
#define OVERLAPCHECKLENGTH 5	// Max distance of two colors to trigger overlap
// How many pixels may come with illegal color after last background color pixel to still accept marker candidate
#define BKGNDOVERLAPCHECKLENGTH 10	// Same for background color

#define LASTDETECTIONCOL_NONE -1

using namespace cv;
using namespace TwoColorCircleMarker;

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

		if (r == g &&  g == b && r <= 64)
		{
			RgbLut[i]=COLORCODE_BLK;
		}
		else if (r == g &&  g == b && r > 64)
		{
			RgbLut[i]=COLORCODE_WHT;
		}
		else if (r >= 160 &&  r >= g+64 && r >= b+32 || (r < 160 && r >= g+32 && r >= b+32))
		{
			RgbLut[i]=COLORCODE_RED;
		}
		else if ((r <= 64 &&  g <= 64 && b >= 64) || (r <= 96 &&  g <= 96 && b >= 128) || (r == 0 &&  g == 0 && b >= 32) || (r == 0 &&  g == 64 && b == 64))	// 3rd condition overrides some blacks...
		{
			RgbLut[i]=COLORCODE_BLU;
		}
		else if ((r <= 64 &&  g >= 96 && b <= 150) || (r <= 96 &&  g >= 128 && b <= 128) || (g < 96 && g >= 64 && g >= r+64 && g >= r+64))
		{
			RgbLut[i]=COLORCODE_GRN;
		}
		else if (r >= 115 && g >= 115 && b >= 115 && abs(r-g)<=35 && abs(r-b)<=35 && abs(g-b)<=35 )
		{
			RgbLut[i]=COLORCODE_WHT;
		}
		else if (r <= 64 &&  g <= 64 && b <= 64 && abs(r-g)<=32 && abs(r-b)<=32 && abs(g-b)<=32)
		{
			RgbLut[i]=COLORCODE_BLK;
		}
	}
}

// ----------------------- Image filtering functions

void FastColorFilter::StartNewFrame()
{
	for (int i=0; i<MAXDETECTIONRECTNUM; i++)
	{
		detectionRects[i].isAlive = false;
	}
	lastUsedDetectionRectIdx = -1;
	nextFreeCandidateRectIdx = 0;
}

void FastColorFilter::RegisterDetection(int row, int colStart, int colEnd)
{
	// Search for detection rects with overlapping with given interval
	bool newRect=true;
	for(int i=0; i<=lastUsedDetectionRectIdx; i++)
	{
		if (!detectionRects[i].isAlive)
		{
			continue;
		}
		// If there is an overlap (no sure gap on either sides)
		if (!(
			// detection is before the rect
			(detectionRects[i].colMin > colEnd) 
			// detection is after the rect
			|| (detectionRects[i].colMax < colStart) ))
		{
			// Update boundaries colMin and colMax
			if (detectionRects[i].colMin > colStart)
			{
				detectionRects[i].colMin = colStart;
			}
			if (detectionRects[i].colMax < colEnd)
			{
				detectionRects[i].colMax = colEnd;
			}

			// Register to keep this rect alive
			detectionRects[i].isDetectedInCurrentRow = true;

			// Should not create a new detection for this
			newRect=false;
		}
	}

	if(newRect)
	{
		// Create new detection rect (find a place for it...)
		bool foundFreeDetectionRect = false;
		for(int i=0; i<MAXDETECTIONRECTNUM; i++)	// Find first !isAlive
		{
			if (!detectionRects[i].isAlive)
			{
				detectionRects[i].isAlive = true;
				detectionRects[i].rowMin = row;
				detectionRects[i].colMin = colStart;
				detectionRects[i].colMax = colEnd;
				detectionRects[i].isDetectedInCurrentRow = true;
				foundFreeDetectionRect = true;
				if (lastUsedDetectionRectIdx < i)
				{
					lastUsedDetectionRectIdx = i;
				}
				break;	// Stop searching
			}
		}
		// If there was no free detectionRect, do nothing...
		if (!foundFreeDetectionRect)
		{
			// Send warning in this case!
			Logger::log(Logger::LOGLEVEL_WARNING, LOG_TAG, "WARNING: FastColorFilter: no enough detection rectangle space to store detections!\n");
		}
	}
}

void FastColorFilter::FinishRow(int rowIdx)
{
	// Create marker candidate from every discontinued detection rectangle
	// TODO: this will not create candidate rect for detections touching the last row of the image!
	for(int i=0; i<=lastUsedDetectionRectIdx; i++)
	{
		if (!detectionRects[i].isAlive)
		{
			continue;
		}
		// Ends in current row or not?
		if (!detectionRects[i].isDetectedInCurrentRow)
		{
			// Not seen anymore, store as candidate
			candidateRects[nextFreeCandidateRectIdx].x = detectionRects[i].colMin;
			candidateRects[nextFreeCandidateRectIdx].y = detectionRects[i].rowMin;
			candidateRects[nextFreeCandidateRectIdx].width = detectionRects[i].colMax-detectionRects[i].colMin;
			candidateRects[nextFreeCandidateRectIdx].height = rowIdx-detectionRects[i].rowMin;

			nextFreeCandidateRectIdx++;
			if (nextFreeCandidateRectIdx >= MAXCANDIDATERECTNUM)
			{
				nextFreeCandidateRectIdx = MAXCANDIDATERECTNUM-1;
				Logger::log(Logger::LOGLEVEL_WARNING, LOG_TAG, "WARNING: FastColorFilter: no enough candidate marker rectangle space!\n");
			}

			// Delete detection rect
			detectionRects[i].isAlive = false;
		}
		else
		{
			// Reset detection status for next row
			detectionRects[i].isDetectedInCurrentRow = false;
		}
	}
}

void FastColorFilter::FindMarkerCandidates(cv::Mat &src, cv::Mat &dst)
{
	// Assert for only 8UC3 input images (BGR)
	assert(src.type() == CV_8UC3);
	// Assert for only 8UC1 output images
	assert(dst.type() == CV_8UC1);
	// Assert dst has same size as src
	assert(src.cols == dst.cols);
	assert(src.rows == dst.rows);

	// Assert masks array (check 1st element)
	assert(overlapMask!=NULL);
	// Assert mask image type is CV_8UC1
	assert(overlapMask->type() == CV_8UC1);
	// Assert mask size
	assert(overlapMask->cols == dst.cols);
	assert(overlapMask->rows == dst.rows);

	uchar colorCode;
	uchar mask0ColorCode = maskColorCode[0];
	uchar mask1ColorCode = maskColorCode[1];

	int lastDetectionCol = LASTDETECTIONCOL_NONE;
	int continuousDetectionStartCol = -1;

	// Mask 0 shift counter:
	//	Set when mask[0] is active. Otherwise decreased until 0.
	//	Used to find areas where mask[1] is active and mask[0] was
	//	also active not so far away.
	//	Processed in every row separately.
	int mask0shiftCounter;
	int backgroundShiftCounter;

	// Init processing of a new frame
	StartNewFrame();

	// Go along every pixel and do the following:
	for (int row=0; row<src.rows; row++)
	{
		// Calculate pointer to the beginning of the current row
		const uchar *ptr = (const uchar *)(src.data + row*src.step);
		// Result pointer
		uchar *resultPtr = (uchar *)(dst.data + row*dst.step);
		// Update mask data pointers
		uchar *overlapDataPtr = (uchar *)(overlapMask->data + row*overlapMask->step);

		// New row, resetting color counters
		mask0shiftCounter = 0;
		backgroundShiftCounter = 0;

		// Reset tracking of continuous detections
		lastDetectionCol = LASTDETECTIONCOL_NONE;
		continuousDetectionStartCol = -1;

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
			bool isMask0Color = (colorCode==mask0ColorCode)?255:0;
			bool isMask1Color = (colorCode==mask1ColorCode)?255:0;
			*overlapDataPtr = 0;

			// Process mask overlap
			if (isMask0Color && backgroundShiftCounter>0)	// mask0 triggered with its own color setting
			{
				// Mask 0 active, trigger checking
				mask0shiftCounter = OVERLAPCHECKLENGTH;
			}
			else
			{
				if (colorCode == backgroundColorCode)	// We are outside a marker...
				{
					// We are not inside a marker
					mask0shiftCounter = 0;	// Warning! This might be caused by pixel error!
					backgroundShiftCounter = BKGNDOVERLAPCHECKLENGTH;
				}
				else
				{
					// Not background and not outer mask color
					if (mask0shiftCounter>0 && backgroundShiftCounter>0)	// If we look for inner mask color at all...
					{
						// Still checking
						if (isMask1Color)	// Inner mask triggers
						{
							// Overlap
							// We do not decrease counters during overlap
							// This helps detection of full overlap and not
							//	only its border...
							*overlapDataPtr = 255;

							// Handle detection collection
							if (lastDetectionCol == LASTDETECTIONCOL_NONE)	// Starting first detection in this row
							{
								// Start new continuous detection
								continuousDetectionStartCol = col;
							}
							else if (lastDetectionCol < col-1)	// There was a gap since last detection
							{
								// Register last continuous detection
								RegisterDetection(row,continuousDetectionStartCol,lastDetectionCol);

								// Start new continuous detection
								continuousDetectionStartCol = col;
							}
							lastDetectionCol = col;
						}
						else
						{
							// Waiting for inner mask to appear...
							backgroundShiftCounter--;
							mask0shiftCounter--;
						}
					}
				} // end else of if colorCode==backgroundColorCode
			}

			overlapDataPtr++;
		}	// end for col
		// Starting new row (if any...)
		if (lastDetectionCol != LASTDETECTIONCOL_NONE)
		{
			// There is still an unregistered detection left
			RegisterDetection(row,continuousDetectionStartCol,lastDetectionCol);
		}

		FinishRow(row);
	}	// end for row
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
