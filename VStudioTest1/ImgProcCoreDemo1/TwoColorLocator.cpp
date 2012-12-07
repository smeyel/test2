#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC1.h"

#include <assert.h>

#include "TimeMeasurementCodeDefines.h"
#include "ConfigManager.h"

using namespace cv;

TwoColorLocator::TwoColorLocator()
{
	fastColorFilter = NULL;
	verboseImage = NULL;
}


void TwoColorLocator::applyOnCC(Mat &redMask, Mat &blueMask)
{
	assert(fastColorFilter!=NULL);

	if (verboseImage != NULL)
	{
		// Verbose image-nek az alapja az eredeti kep
		//srcCC.copyTo(*verboseImage);	// NE CSAPJUK FELUL, AMI MAR RAJTA VAN!
	}

	// --- Calculate integral images of masks
	Mat h1integral, h2integral;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::IntegralImages);
	integral(redMask,h1integral,CV_32S);
	integral(blueMask,h2integral,CV_32S);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::IntegralImages);

	CV_Assert(redMask.rows == blueMask.rows);
	CV_Assert(blueMask.cols == blueMask.cols);

	// --- Az integral image alapjan a sorok es oszopok pixelertek osszegeit szamoljuk ki minden maszkra
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ProcessIntegralImages);
	int *h1rowOccNums = new int[redMask.rows];
	int *h1colOccNums = new int[redMask.cols];
	int *h2rowOccNums = new int[blueMask.rows];
	int *h2colOccNums = new int[blueMask.cols];
	int rowmax, colmax;
	// H1 mask
	getOccurranceNumbers(h1integral,h1rowOccNums,h1colOccNums,rowmax,colmax);
	// H2 mask
	getOccurranceNumbers(h2integral,h2rowOccNums,h2colOccNums,rowmax,colmax);

	// Peremen megjelenitjuk a pixel darabszamokat az egyes maszkok szerint
	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,h1rowOccNums,redMask.rows,rowmax/50,Scalar(0,0,255),Right);
		drawValuesOnMargin(*verboseImage,h1colOccNums,redMask.cols,colmax/50,Scalar(0,0,255),Bot);
		drawValuesOnMargin(*verboseImage,h2rowOccNums,blueMask.rows,rowmax/50,Scalar(255,0,0),Left);
		drawValuesOnMargin(*verboseImage,h2colOccNums,blueMask.cols,colmax/50,Scalar(255,0,0),Top);
	}

	// --- A ket maszk osszevonasa egybe (hol teljesul mindketto "peremfeltetele")
	int rownum = blueMask.rows;
	int colnum = blueMask.cols;
	int *mergedRowOccNums = new int[rownum];
	int *mergedColOccNums = new int[colnum];
	// Calculating minimum of two vectors (merging)
	int mergedRowMax = 0;
	int mergedColMax = 0;
	for(int i=0; i<rownum; i++)
	{
		mergedRowOccNums[i] = h1rowOccNums[i]<h2rowOccNums[i] ? h1rowOccNums[i] : h2rowOccNums[i];
		if (mergedRowMax<mergedRowOccNums[i])
			mergedRowMax = mergedRowOccNums[i];
	}
	for(int i=0; i<colnum; i++)
	{
		mergedColOccNums[i] = h1colOccNums[i]<h2colOccNums[i] ? h1colOccNums[i] : h2colOccNums[i];
		if (mergedColMax<mergedColOccNums[i])
			mergedColMax = mergedColOccNums[i];
	}

	delete h1rowOccNums;
	delete h1colOccNums;
	delete h2rowOccNums;
	delete h2colOccNums;

	if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
	{
		drawValuesOnMargin(*verboseImage,mergedRowOccNums,blueMask.rows,mergedRowMax/50,Scalar(0,255,255),Right);
		drawValuesOnMargin(*verboseImage,mergedColOccNums,blueMask.cols,mergedColMax/50,Scalar(0,255,255),Bot);
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ProcessIntegralImages);

	resultRectangles.clear();

	getMarkerCandidateRectanges(mergedRowOccNums, mergedColOccNums, rownum, colnum,
		mergedRowMax, mergedColMax, 0.2, resultRectangles, verboseImage);

	delete mergedRowOccNums;
	delete mergedColOccNums;
}


// ----------------------- HSV version

/*void TwoColorLocator::apply(Mat &srcBGR)
{
	if (verboseImage != NULL)
	{
		// Verbose image-nek az alapja az eredeti kep
		srcBGR.copyTo(*verboseImage);
	}

	// --- Filter colors, create masks
	Mat h1mask, h2mask;
	twoColorFilter->apply(srcBGR,h1mask,h2mask);

	// --- Calculate integral images of masks
	Mat h1integral, h2integral;
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::IntegralImages);
	integral(h1mask,h1integral,CV_32S);
	integral(h2mask,h2integral,CV_32S);
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::IntegralImages);

	CV_Assert(h1mask.rows == h2mask.rows);
	CV_Assert(h1mask.cols == h2mask.cols);

	// --- Az integral image alapjan a sorok es oszopok pixelertek osszegeit szamoljuk ki minden maszkra
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::ProcessIntegralImages);
	int *h1rowOccNums = new int[h1mask.rows];
	int *h1colOccNums = new int[h1mask.cols];
	int *h2rowOccNums = new int[h2mask.rows];
	int *h2colOccNums = new int[h2mask.cols];
	int rowmax, colmax;
	// H1 mask
	getOccurranceNumbers(h1integral,h1rowOccNums,h1colOccNums,rowmax,colmax);
	// H2 mask
	getOccurranceNumbers(h2integral,h2rowOccNums,h2colOccNums,rowmax,colmax);

	// Peremen megjelenitjuk a pixel darabszamokat az egyes maszkok szerint
	if (verboseImage != NULL && ConfigManager::Current()->twocolorlocator_verbose)
	{
		drawValuesOnMargin(*verboseImage,h1rowOccNums,h1mask.rows,rowmax/50,Scalar(0,0,255),Right);
		drawValuesOnMargin(*verboseImage,h1colOccNums,h1mask.cols,colmax/50,Scalar(0,0,255),Bot);
		drawValuesOnMargin(*verboseImage,h2rowOccNums,h2mask.rows,rowmax/50,Scalar(255,0,0),Left);
		drawValuesOnMargin(*verboseImage,h2colOccNums,h2mask.cols,colmax/50,Scalar(255,0,0),Top);
	}

	// --- A ket maszk osszevonasa egybe (hol teljesul mindketto "peremfeltetele")
	int rownum = h2mask.rows;
	int colnum = h2mask.cols;
	int *mergedRowOccNums = new int[rownum];
	int *mergedColOccNums = new int[colnum];
	// Calculating minimum of two vectors (merging)
	int mergedRowMax = 0;
	int mergedColMax = 0;
	for(int i=0; i<rownum; i++)
	{
		mergedRowOccNums[i] = h1rowOccNums[i]<h2rowOccNums[i] ? h1rowOccNums[i] : h2rowOccNums[i];
		if (mergedRowMax<mergedRowOccNums[i])
			mergedRowMax = mergedRowOccNums[i];
	}
	for(int i=0; i<colnum; i++)
	{
		mergedColOccNums[i] = h1colOccNums[i]<h2colOccNums[i] ? h1colOccNums[i] : h2colOccNums[i];
		if (mergedColMax<mergedColOccNums[i])
			mergedColMax = mergedColOccNums[i];
	}

	delete h1rowOccNums;
	delete h1colOccNums;
	delete h2rowOccNums;
	delete h2colOccNums;

	if (verboseImage != NULL && ConfigManager::Current()->twocolorlocator_verbose)
	{
		drawValuesOnMargin(*verboseImage,mergedRowOccNums,h2mask.rows,mergedRowMax/50,Scalar(0,255,255),Right);
		drawValuesOnMargin(*verboseImage,mergedColOccNums,h2mask.cols,mergedColMax/50,Scalar(0,255,255),Bot);
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::ProcessIntegralImages);

	resultRectangles.clear();

	getMarkerCandidateRectanges(mergedRowOccNums, mergedColOccNums, rownum, colnum,
		mergedRowMax, mergedColMax, 0.2, resultRectangles, verboseImage);

	delete mergedRowOccNums;
	delete mergedColOccNums;
} */

// Az integral image peremosszegeit szamolja ki
void TwoColorLocator::getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax)
{
	// TODO: ez most lassu modszer a hozzaferesre! Majd fel kell gyorsitani...
	int rowNumMinus1 = srcIntegral.rows-1;
	int colNumMinus1 = srcIntegral.cols-1;
	rowmax = 0;
	colmax = 0;
	// Warning: last row and column is omitted in result...
	for(int row=0; row<rowNumMinus1; row++)
	{
		int current = srcIntegral.at<int>(row, colNumMinus1);
		int next = srcIntegral.at<int>(row+1, colNumMinus1);
		int diff = next - current;
		rowOccNums[row] = diff;
		if (rowmax < diff)
			rowmax=diff;
	}
	for(int col=0; col<colNumMinus1; col++)
	{
		int current = srcIntegral.at<int>(rowNumMinus1, col);
		int next = srcIntegral.at<int>(rowNumMinus1, col+1);
		int diff = next - current;
		colOccNums[col] = diff;
		if (colmax < diff)
			colmax=diff;
	}
}

// Megjelenites: integral image peremosszegek a margokon
void TwoColorLocator::drawValuesOnMargin(Mat &img, int *values, int valueNum,
	int scalingDivider, Scalar color, LocationEnum loc)
{
	if (scalingDivider==0)
	{
		return;
	}

	for(int i=0; i<valueNum; i++)
	{
		int value = values[i] / scalingDivider;
		CvPoint start, end;
		switch(loc)
		{
		case Top:
			start = cvPoint(i,0);
			end = cvPoint(i,value);
			break;
		case Bot:
			start = cvPoint(i,img.rows-1);
			end = cvPoint(i,img.rows-1-value);
			break;
		case Left:
			start = cvPoint(0,i);
			end = cvPoint(value,i);
			break;
		case Right:
			start = cvPoint(img.cols-1,i);
			end = cvPoint(img.cols-1-value,i);
			break;
		}

		line(img,start,end,color);
	}
}

// A peremosszegekbol threshold alapjan letrehozza az eselyes negyzetek listjajat.
void TwoColorLocator::getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax,
	double thresholdRate, std::list<CvRect> &resultRectangles, Mat *verboseImage)
{
//	CV_Assert(rownum == verboseImage->rows);
//	CV_Assert(colnum == verboseImage->cols);
	TimeMeasurement::instance.start(TimeMeasurementCodeDefs::GetCandidateRectangles);

	// Thresholding for 30%
	int rowValThreshold = rowMax * thresholdRate;
	int colValThreshold = colMax * thresholdRate;

	std::list<Scalar> rowIntervalList;
	int prev = 0;
	int currentIntervalBegin = 0;
	for (int i=0; i<rownum; i++)
	{
		if (rowVals[i] >= rowValThreshold)
		{
			// Now above threshold
			if (prev < rowValThreshold)
			{
				// previously below threshold, now entering new interval
				currentIntervalBegin = i;
			}
		}
		else
		{
			// Now below threshold
			if (prev >= rowValThreshold)
			{
				// previously above threshold, now exiting new interval
				Scalar newInterval = Scalar(currentIntervalBegin, i);
				rowIntervalList.push_back(newInterval);

				if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
				{
					rectangle(*verboseImage,cvPoint(0,currentIntervalBegin),cvPoint(20,i),Scalar(0,255,255));
				}
			}
		}

		prev = rowVals[i];
	}

	std::list<Scalar> colIntervalList;
	prev = 0;
	currentIntervalBegin = 0;
	for (int i=0; i<colnum; i++)
	{
		if (colVals[i] >= colValThreshold)
		{
			// Now above threshold
			if (prev < colValThreshold)
			{
				// previously below threshold, now entering new interval
				currentIntervalBegin = i;
			}
		}
		else
		{
			// Now below threshold
			if (prev >= colValThreshold)
			{
				// previously above threshold, now exiting new interval
				Scalar newInterval = Scalar(currentIntervalBegin, i);
				colIntervalList.push_back(newInterval);

				if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
				{
					rectangle(*verboseImage,cvPoint(currentIntervalBegin,0),cvPoint(i,20),Scalar(0,255,255));
				}
			}
		}

		prev = colVals[i];
	}

	// Combine intervals into rectangles
	for (	std::list<Scalar>::iterator rowIt = rowIntervalList.begin();
			rowIt != rowIntervalList.end();
			rowIt++ )
	{
		int y = (*rowIt)[0];
		int height = (*rowIt)[1] - y;
		for (	std::list<Scalar>::iterator colIt = colIntervalList.begin();
				colIt != colIntervalList.end();
				colIt++ )
		{
			int x = (*colIt)[0];
			int width = (*colIt)[1] - x;

			CvRect newRect = cvRect(x,y,width,height);

			resultRectangles.push_back(newRect);

			// visualize
			if (verboseImage != NULL && ConfigManager::Current()->verboseTwoColorLocator)
			{
				rectangle(*verboseImage,newRect,Scalar(255,255,255));
			}
		}
	}
	TimeMeasurement::instance.finish(TimeMeasurementCodeDefs::GetCandidateRectangles);
}
