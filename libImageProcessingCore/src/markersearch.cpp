#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "marker.h"
#include "markersearch.h"

typedef enum _Location { Top, Bot, Left, Right } LocationEnum;

Mat h1mask, h2mask, smask;
Mat hsv, h, s, tmp;

// Itt van, azert, hogy a maszkokba bele tudjon nezni kattintasra...
void mouse_callback(int eventtype, int x, int y, int flags, void *param)
{
	if (eventtype == CV_EVENT_LBUTTONDOWN)
	{
		int hue = h.at<uchar>(y, x);
		int sat = s.at<uchar>(y, x);
		int h1m = h1mask.at<uchar>(y, x);
		int h2m = h2mask.at<uchar>(y, x);
		int sm = smask.at<uchar>(y, x);

		cout << "Click at " << x << "-" << y << ", h=" << hue << ", sat=" << sat << 
			", h1m=" << h1m << ", h2m=" << h2m << ", sm=" << sm << endl;
	}
}

// Eloallitja a BGR kepbol a filtereknek megfelelo Hue es Sat kepeket es maszkokat.
void filterBGRInHS(Mat *bgr,
	FilterThreshold hue1Mask, FilterThreshold hue2Mask, FilterThreshold satMask)
{
	// Extract hue channel
	cvtColor(*bgr, hsv, CV_BGR2HSV);
	h.create(hsv.size(), CV_8UC1);
	s.create(hsv.size(), CV_8UC1);
	Mat out[] = {h,s};	// data is not copied, only the headers...
	int fromTo[] = {0,0, 1,1};	// 0->0 (HSV.H->H), 1->1 (HSV.S->S)
	mixChannels(&hsv,1,out,2,fromTo,2); // src, 1 src mtx, dst, 2 dst mtx, mapping, 2 pairs

	// SAT (lowpass)
	threshold(s,smask,satMask.low,255,THRESH_BINARY);

	// Thresholds: preserved values: loTh <= x <= hiTh, else zeroed...
	// Apply SAT filter to h1 and h2 masks
	// HUE 1 (bandpass)
	inRange(h, Scalar(hue1Mask.low), Scalar(hue1Mask.high), tmp);
	min(tmp,smask,h1mask);
	// HUE 2 (bandpass)
	inRange(h, Scalar(hue2Mask.low), Scalar(hue2Mask.high), tmp);
	min(tmp,smask,h2mask);

	//addWeighted(h1mask,0.5,h2mask,0.5,0,*res);
}

// Az integral image peremosszegeit szamolja ki
void getOccurranceNumbers(Mat &srcIntegral, int* rowOccNums, int *colOccNums, int &rowmax, int &colmax)
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
void drawValuesOnMargin(Mat &img, int *values, int valueNum,
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
void getMarkerCandidateRectanges(int *rowVals, int *colVals, int rownum, int colnum, int rowMax, int colMax,
	double thresholdRate, std::list<CvRect> &resultRectangles, Mat *imgForDebug)
{
	CV_Assert(rownum == imgForDebug->rows);
	CV_Assert(colnum == imgForDebug->cols);

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

				rectangle(*imgForDebug,cvPoint(0,currentIntervalBegin),cvPoint(20,i),Scalar(0,255,255));
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

				rectangle(*imgForDebug,cvPoint(currentIntervalBegin,0),cvPoint(i,20),Scalar(0,255,255));
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
			if (imgForDebug != NULL)
			{
				rectangle(*imgForDebug,newRect,Scalar(255,255,255));
			}
		}
	}
}

// Frame feldolgozasa, atfogo fuggveny
// Calls: filterBGRInHS, getOccurranceNumbers, drawValuesOnMargin (csak megjelenites),
//	getMarkerCandidateRectanges, readMarkerCode
void processFrame(Mat *input, Mat *result)
{
	// Filter for given colors, create mask images
	FilterThreshold h1filter,h2filter,sfilter;
	// RED filter (acryl: 176-178)
	h1filter.low=168;	// 173
	h1filter.high=180;	// 181
	// BLUE filter (acryl: 108-110)
	h2filter.low=125;	// 105
	h2filter.high=145;	// 113
	// SAT filter (acryl: 160) filc,w910i:90
	sfilter.low=50;	// 160 forN8, 90 forW910i
	sfilter.high=90;	// not used...

	filterBGRInHS(input, h1filter, h2filter, sfilter);
//	imshow("Red",h1mask);
//	imshow("Blu",h2mask);
//	imshow("Sat",smask);

	// Calculate integral images of masks
	Mat h1integral, h2integral;
	integral(h1mask,h1integral,CV_32S);
	integral(h2mask,h2integral,CV_32S);
	
	// === Search for markers
	// Calculate occurrance numbers for individual columns of image (for both hue masks separately)

	// Input image is base for output visualization
	input->copyTo(*result);

	CV_Assert(h1mask.rows == h2mask.rows);
	CV_Assert(h1mask.cols == h2mask.cols);
	int *h1rowOccNums = new int[h1mask.rows];
	int *h1colOccNums = new int[h1mask.cols];
	int *h2rowOccNums = new int[h2mask.rows];
	int *h2colOccNums = new int[h2mask.cols];
	int rowmax, colmax;
	// H1 mask
	getOccurranceNumbers(h1integral,h1rowOccNums,h1colOccNums,rowmax,colmax);
	drawValuesOnMargin(*result,h1rowOccNums,h1mask.rows,rowmax/50,Scalar(0,0,255),Right);
	drawValuesOnMargin(*result,h1colOccNums,h1mask.cols,colmax/50,Scalar(0,0,255),Bot);
	// H2 mask
	getOccurranceNumbers(h2integral,h2rowOccNums,h2colOccNums,rowmax,colmax);
	drawValuesOnMargin(*result,h2rowOccNums,h2mask.rows,rowmax/50,Scalar(255,0,0),Left);
	drawValuesOnMargin(*result,h2colOccNums,h2mask.cols,colmax/50,Scalar(255,0,0),Top);

	// Merge H1 and H2 masks: locations of co-occurrances
	int rownum = h2mask.rows;
	int colnum = h2mask.cols;
	int *mergedRowOccNums = new int[rownum];
	int *mergedColOccNums = new int[colnum];

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

	drawValuesOnMargin(*result,mergedRowOccNums,h2mask.rows,mergedRowMax/50,Scalar(0,255,255),Right);
	drawValuesOnMargin(*result,mergedColOccNums,h2mask.cols,mergedColMax/50,Scalar(0,255,255),Bot);

	std::list<CvRect> resultRectangles;

	getMarkerCandidateRectanges(mergedRowOccNums, mergedColOccNums, rownum, colnum,
		mergedRowMax, mergedColMax, 0.2, resultRectangles, result);

	// Mask h with smask...	(TODO: ezt igazabol mar joval korabban meg lehetne tenni...)
	bitwise_and(h,smask,h);
	//imshow(wndTmp,h);

	// Markerek leolvasasa
	for (std::list<CvRect>::iterator rectIt = resultRectangles.begin();
		 rectIt != resultRectangles.end();
		 rectIt++)
	{
		Marker newMarker;
		readMarkerCode(h,*rectIt,newMarker,result);
	}


}
