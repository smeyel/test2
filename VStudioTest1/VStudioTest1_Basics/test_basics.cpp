#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include <time.h>

using namespace cv;
using namespace std;

void ShowMatrix32FC1Content(Mat mtx);
void ShowMatrix8UC1Content(Mat mtx);

void do_test1();

int main()
{
	do_test1();
}

void do_test1()
{
	Mat alapkep = imread("../../../inputmedia/alapkep.jpg");
	Mat minta = imread("../../../inputmedia/minta.png");

	if ((alapkep.data == NULL) || (minta.data == NULL))
	{
		printf("Cannot load file...\n");
	}
	else
	{
		imshow("Minta",minta);

		int resultW = alapkep.cols - minta.cols + 1;
		int resultH = alapkep.rows - minta.rows + 1;
		printf("Eredmeny merete: w%d, h%d\n",resultW,resultH);

		Mat eredmeny(resultW,resultH,CV_32FC1);

		printf("Matching...");
		matchTemplate(alapkep,minta,eredmeny,CV_TM_SQDIFF);
		printf("OK\n");

		printf("Normalizing...");
		normalize(eredmeny,eredmeny,0,1,NORM_MINMAX, -1, Mat());
		printf("OK\n");

		printf("Searching for minimum SqrDiff...");
		double minVal, maxVal;
		Point minLoc,maxLoc,matchLoc;
		minMaxLoc(eredmeny,&minVal,&maxVal,&minLoc,&maxLoc,Mat());
		matchLoc = minLoc;
		printf("OK\n");

		rectangle(eredmeny, matchLoc,
			Point(matchLoc.x+minta.cols,matchLoc.y+minta.rows),
			Scalar::all(0),2,8,0);

		rectangle(alapkep, matchLoc,
			Point(matchLoc.x+minta.cols,matchLoc.y+minta.rows),
			Scalar::all(0),2,8,0);

		imshow("Eredmeny-alapkepen",alapkep);
		imshow("Eredmeny",eredmeny);

		//ShowMatrix32FC1Content(eredmeny);
	}
	waitKey(0);
	return;
}

void ShowMatrix32FC1Content(Mat mtx)
{
	int resultW = mtx.cols;
	int resultH = mtx.rows;
	printf("32FC1 Eredmeny merete: w%d, h%d\n",resultW,resultH);

	for (int r=0; r<resultH; r++)
	{
		for (int c=0; c<resultW; c++)
		{
			printf("c%d, r%d: ",c,r);
			float pointValue = mtx.at<float>(r,c);	// WRN koordinata sorrend!
			printf("%3.2f\n",pointValue);
		}
		printf("\n");
	}
	printf("ShowMatrixContent done.");
}

void ShowMatrix8UC1Content(Mat mtx)
{
	int resultW = mtx.cols;
	int resultH = mtx.rows;
	printf("8UC1 Eredmeny merete: w%d, h%d\n",resultW,resultH);

	for (int r=0; r<resultH; r++)
	{
		for (int c=0; c<resultW; c++)
		{
			printf("c%d, r%d: ",c,r);
			unsigned int pointValue = mtx.at<unsigned int>(r,c);	// WRN koordinata sorrend!
			printf("%u\n",pointValue);
		}
		printf("\n");
	}
	printf("ShowMatrixContent done.");
}