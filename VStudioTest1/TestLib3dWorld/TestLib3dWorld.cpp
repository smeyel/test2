#include <iostream>
#include <sstream>
#include <time.h>

/*#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp> */
#include <opencv2/highgui/highgui.hpp>

#include "VideoInputGeneric.h"
//#include "VideoInputPs3Eye.h"

#include "camera.h"
#include "ray.h"
#include "ray2d.h"

using namespace cv;
using namespace std;

Mat cameraMatrix, distCoeffs;

void show(char *msg, Matx41f value)
{
	cout << msg << ": ";
	for(int i=0; i<4; i++)
	{
		cout << value.val[i] << " ";
	}
	cout << endl;
}

void show(char *msg, Matx44f value)
{
	cout << msg << ":" << endl;
	int i=0;
	for(int r=0; r<4; r++)
	{
		for(int c=0; c<4; c++)
		{
			cout << value.val[i++] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

class Chessboard
{
public:
	vector<Point3f> corners;
	int squareSize;
	Size boardSize;

	Chessboard(Size iBoardSize, int iSquareSize)
	{
		squareSize = iSquareSize;
		boardSize = iBoardSize;
		calcCorners(boardSize, squareSize);
	}

	void calcCorners(Size boardSize, int squareSize)
	{
		for( int i = 0; i < boardSize.height; ++i )
			for( int j = 0; j < boardSize.width; ++j )
				corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
	}
};

class ChessboardDetector
{
	Size boardSize;
	int squareSize;

public:
	ChessboardDetector(Size iBoardSize, int iSquareSize)
	{
		found = false;
		boardSize = iBoardSize;
		squareSize = iSquareSize;
		pointBuf.resize(boardSize.width * boardSize.height);
	}

	bool found;
	vector<Point2f> pointBuf;

	bool findChessboardInFrame(Mat& frame)
	{
		//int succeses = 0;

		int cornersDetected = 0;
		vector<CvPoint2D32f> corners;
		IplImage dst_img = frame;
		int sumCornerNum = boardSize.width * boardSize.height;
		corners.resize(sumCornerNum);
		int result = cvFindChessboardCorners(&dst_img, cvSize(boardSize.width, boardSize.height), &corners[0], &cornersDetected, 
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
		found = (cornersDetected == sumCornerNum);

		if (found)                // If done with success,
		{
			// Copy results into pointBuf
			for(int i=0; i<sumCornerNum; i++)
			{
				pointBuf[i]=Point2f(corners[i]);
			}

			// improve the found corners' coordinate accuracy for chessboard
			Mat viewGray;
			cvtColor(frame, viewGray, CV_BGR2GRAY);
/*			cornerSubPix( viewGray, pointBuf, Size(11,11),
				Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 )); */

			// Draw the corners.
			drawChessboardCorners( frame, boardSize, Mat(pointBuf), found );

			cout << "(Chessboard detected in image.)" << endl;
		}

		return found;
	}
};


void test1()
{
		// Mat M = (Mat_<double>(3,3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
	Camera camW;

	camW.loadCalibrationData("test1.xml");
/*	for (int r=0; r<distCoeffs.rows; r++)
	{
		for (int c=0; c<distCoeffs.cols; c++)
		{
			cout << "r"<<r<<"c"<<c<<":"<<distCoeffs.at<double>(r,c)<<endl;
		}
	} */

	// Create a camera in the center of the world coordinate system,
	//	looking in the direction of the Z axis
	// fx=fy=100, cx=cy=0
	cameraMatrix = 	(Mat_<double>(3,3) << 100,0,0,   0,100,0,   0,0,1);

	camW.cameraID = 1;
	camW.setCameraMatrix(cameraMatrix);
	camW.T = Matx44f(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);

	// Create a second (testing) camera
	Camera camT;
	camT.cameraID = 2;
	camT.setCameraMatrix(cameraMatrix);
	camT.T = Matx44f(1,0,0,0,   0,0,-1,1000,   0,1,0,0,   0,0,0,1);

	// ------------------ Test Cam ---> World coordinate transformations
	Matx41f pXT = Matx41f(1,0,0,1);	// point, x unit vector, CamT coord.sys.
	show("pXT (should be 1 0 0 1)",pXT);
	Matx41f pYT = Matx41f(0,1,0,1);	// point, y unit vector, CamT coord.sys.
	show("pYT (should be 0 1 0 1)",pYT);
	Matx41f pZT = Matx41f(0,0,1,1);	// point, z unit vector, CamT coord.sys.
	show("pZT (should be 0 0 1 1)",pZT);

	// Point Cam ---> World
	Matx41f pXW = camT.pointCam2World(pXT);
	show("pXW (should be 1 1000 0 1)",pXW);
	Matx41f pYW = camT.pointCam2World(pYT);
	show("pYW (should be 0 1000 1 1)",pYW);
	Matx41f pZW = camT.pointCam2World(pZT);
	show("pZW (should be 0 999 0 1)",pZW);

	// Point World ---> Cam
	pXT = camT.pointWorld2Cam(pXW);
	show("pXT (should be 1 0 0 1)",pXT);
	pYT = camT.pointWorld2Cam(pYW);
	show("pYT (should be 0 1 0 1)",pYT);
	pZT = camT.pointWorld2Cam(pZW);
	show("pZT (should be 0 0 1 1)",pZT);

	// Testing Ray Cam <---> World
	Ray r1T;
	r1T.cameraID = camT.cameraID;
	r1T.originalCameraID = camT.cameraID;
	r1T.A = pXT;
	r1T.B = Matx41f(1,1,1,1);
	r1T.show("r1T");

	Ray r1W = camT.rayCam2World(r1T);
	r1W.show("r1W");
	r1T = camT.rayWorld2Cam(r1W);
	r1T.show("r1T");

	// Testing Point Cam -> Image
	Matx41f p1W = Matx41f(20,-10,200,1);	// CamW sees it 200 pixel forward, 10 pixel left and 20 pixels up
	show("pZW (should be 20 -10 200 1)",p1W);
	Point2f p1I = camW.pointCam2Img(p1W);
	cout << "p1I (*): " << p1I.x << " " << p1I.y << endl;	// Should be left and twice more up...

	// Testing Point Img -> Ray
	r1W = camW.pointImg2Cam(p1I);
	r1W.show("r1W (should be from origo towards 20 -10 200");

	// Testing Ray Cam -> Img
	Point2f r1I_orig = camW.rayOrigCam2Img(r1W);
	cout << "r1I_orig (should be the same as the one marked with *) :" << r1I_orig.x << " " << r1I_orig.y << endl;
	r1T = camT.rayWorld2Cam(r1W);
	r1T.show("r1T");
	Ray2D r1I = camT.rayCam2Img(r1T);
	r1I = camT.rayCam2Img(r1T);
	r1I.show("r1W: ");

	cout << endl;
}

void findChessboardAndShowExtrParams(Mat& frame, Camera cam, ChessboardDetector detector, Chessboard chessboard)
{
	// Find chessboard and calculate extr. params
	if (detector.findChessboardInFrame(frame))
	{
//		cout << "Now drawing chessboard corners..." << endl;
		drawChessboardCorners(frame,Size(9,6),detector.pointBuf,true);

//		cout << "Now calculating extrinsic params..." << endl;
		cam.loadCalibrationData("test1.xml");
		cam.calculateExtrinsicParams(chessboard.corners,detector.pointBuf);
//		show("Extr.params T=",cam.T);

		// Show extr. params on frame
		char txt[50];
		for(int i=0; i<16; i++)
		{
			sprintf(txt, "%4.2lf",cam.T.val[i] );
			putText( frame, string(txt), cvPoint( 25+(i%4)*75, 20+(i/4)*20 ), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,0) );
		}



	}
}



/** Creates a camera, starts an image capture and calculates the extrinsic parameters if a chessboard becomes visible.
	May also be used to calibrate the intrinsic parameters (or they can be loaded form a file.)
*/
void test_extrinsics()
{
	VideoInputGeneric cam1;
	cam1.init(0);

	// The <ESC> key will exit the program
	Mat frame(480,640,CV_8UC4);
	char key;
	bool running = true;
	Chessboard chessboard(Size(9,6),50);
	ChessboardDetector detector(Size(9,6),50);
	Camera cam;
	while(running)
	{
		cam1.captureFrame(frame);

		findChessboardAndShowExtrParams(frame, cam, detector, chessboard);
		imshow("Default video input",frame);	// To display results...

		key = waitKey(2000);
		if (key==27)
		{
			running=false;
		}
	}
	cam1.release();
}

void main()
{
	//test1();
	test_extrinsics();
}
