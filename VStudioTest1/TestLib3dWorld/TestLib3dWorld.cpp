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

/** Creates a camera, starts an image capture and calculates the extrinsic parameters if a chessboard becomes visible.
	May also be used to calibrate the intrinsic parameters (or they can be loaded form a file.)
*/
void test_extrinsics()
{
	VideoInputGeneric cam1;
	cam1.init(0);

	// The <ESC> key will exit the program
	Mat image1(480,640,CV_8UC4);
	char key;
	bool running = true;
	while(running)
	{
		cam1.captureFrame(image1);

		imshow("Default video input",image1);

		key = cvWaitKey(25);
		
		switch (key)
		{
			case 27:
				running = false;
				break;
			case 'e':
				// Find chessboard and calculate extr. params

				break;
			default:
				cout << "(Use ESC to exit, 'e' to find chessboard and calculate extrinsic parameters.)" << endl;
				break;
		}
	}
}

void main()
{
	//test1();
	test_extrinsics();
}
