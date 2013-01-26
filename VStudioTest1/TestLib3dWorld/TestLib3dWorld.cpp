#include <iostream>
#include <sstream>
//#include <stdlib.h>
#include <time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//#include "VideoInputGeneric.h"
#include "VideoInputPs3Eye.h"

//#include "chessboard.h"
#include "chessboarddetector.h"
#include "showhelper.h"

#include "camera.h"
#include "ray.h"
#include "ray2d.h"

using namespace cv;
using namespace std;

Mat cameraMatrix, distCoeffs;

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
	ShowHelper::show("pXT (should be 1 0 0 1)",pXT);
	Matx41f pYT = Matx41f(0,1,0,1);	// point, y unit vector, CamT coord.sys.
	ShowHelper::show("pYT (should be 0 1 0 1)",pYT);
	Matx41f pZT = Matx41f(0,0,1,1);	// point, z unit vector, CamT coord.sys.
	ShowHelper::show("pZT (should be 0 0 1 1)",pZT);

	// Point Cam ---> World
	Matx41f pXW = camT.pointCam2World(pXT);
	ShowHelper::show("pXW (should be 1 1000 0 1)",pXW);
	Matx41f pYW = camT.pointCam2World(pYT);
	ShowHelper::show("pYW (should be 0 1000 1 1)",pYW);
	Matx41f pZW = camT.pointCam2World(pZT);
	ShowHelper::show("pZW (should be 0 999 0 1)",pZW);

	// Point World ---> Cam
	pXT = camT.pointWorld2Cam(pXW);
	ShowHelper::show("pXT (should be 1 0 0 1)",pXT);
	pYT = camT.pointWorld2Cam(pYW);
	ShowHelper::show("pYT (should be 0 1 0 1)",pYT);
	pZT = camT.pointWorld2Cam(pZW);
	ShowHelper::show("pZT (should be 0 0 1 1)",pZT);

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
	ShowHelper::show("pZW (should be 20 -10 200 1)",p1W);
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

vector<Ray> rays;

void addRayFromCameraToOrigin(Camera &cam)
{
	// Create Ray (3D)
	Ray *newRay = new Ray();	// TODO: should be deleted at exit
	newRay->cameraID = CAMID_WORLD;	// Now in world coordinates
	newRay->originalCameraID = CAMID_WORLD;	// Created in world coordinates
	newRay->originalImageLocation = Point2f(0.0,0.0);	// dummy data
	float x = cam.T.val[3] / cam.T.val[15] / 2.0;	// Half way from origin to camera
	float y = cam.T.val[7] / cam.T.val[15] / 2.0;
	float z = cam.T.val[11] / cam.T.val[15] / 2.0;
	newRay->A = Matx41f(x,y,z,1);	// Starts from half way to the the camera location (from origin)
	newRay->B = Matx41f(0,0,0,1);	// Towards the origin
	// Add to ray list
	rays.push_back(*newRay);
}

void showRaysOnImage(Camera& cam, Mat& frame)
{
	for(int i=0; i<rays.size(); i++)
	{
		Ray& rayWorld = rays[i];
		Ray rayCam = cam.rayWorld2Cam(rayWorld);
		if(rayCam.A.val[2]<2)
		{
			rayCam.A.val[2] = 2;
		}
		Ray2D ray2D = cam.rayCam2Img(rayCam);
		line(frame,ray2D.A,ray2D.B,Scalar(255,0,0));
	}
}

void findChessboardAndAddRays(Mat& frame, Camera& cam, ChessboardDetector& detector, bool addRays=false)
{
	if (detector.findChessboardInFrame(frame))
	{
		drawChessboardCorners(frame,Size(9,6),detector.pointBuf,true);
		cam.calculateExtrinsicParams(detector.chessboard.corners,detector.pointBuf);
		char txt[50];
		for(int i=0; i<16; i++)
		{
			sprintf(txt, "%4.2lf",cam.T.val[i] );
			putText( frame, string(txt), cvPoint( 25+(i%4)*75, 20+(i/4)*20 ), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,0) );
		}
		if (addRays)
		{
			addRayFromCameraToOrigin(cam);
		}
		showRaysOnImage(cam,frame);
	}
}

/** Creates a camera, starts an image capture and calculates the extrinsic parameters if a chessboard becomes visible.
	After every 2s, creates a ray pointing towards the chessboard. By moving the camera, previous rays remain visible.
*/
void test_rayshow()
{
	VideoInputPs3Eye videoInput0;
	videoInput0.init(0);
	VideoInputPs3Eye videoInput1;
	videoInput1.init(1);

	// The <ESC> key will exit the program
	Mat frame0(480,640,CV_8UC4);
	Mat frame1(480,640,CV_8UC4);
	Mat frameUndistort0(480,640,CV_8UC4);
	Mat frameUndistort1(480,640,CV_8UC4);
	char key;
	bool running = true;
	//Chessboard chessboard(Size(9,6),50);
	ChessboardDetector detector(Size(9,6),50);
	Camera cam0;
	Camera cam1;
	cam0.cameraID=0;
	cam1.cameraID=1;
	cam0.isStationary = false;
	cam1.isStationary = true;
	cout << "Warning, CAM1 has to be stationary!" << endl;
	cam0.loadCalibrationData("test1.xml");
	cam1.loadCalibrationData("test1.xml");
	while(running)
	{
		videoInput0.captureFrame(frame0);
		videoInput1.captureFrame(frame1);

		cam0.undistortImage(frame0,frameUndistort0);
		cam1.undistortImage(frame1,frameUndistort1);

		findChessboardAndAddRays(frameUndistort0, cam0, detector,true);
		findChessboardAndAddRays(frameUndistort1, cam1, detector);

		imshow("Cam 0 (ray source, undistorted)",frameUndistort0);	// To display results...
		imshow("Cam 1 (view only, undistorted)",frameUndistort1);	// To display results...

		key = waitKey(25);
		if (key==27)
		{
			running=false;
		}
	}
	videoInput0.release();
	videoInput1.release();
}


void main()
{
	test_rayshow();
}
