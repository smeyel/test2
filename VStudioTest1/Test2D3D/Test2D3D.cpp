#include <iostream>
#include <sstream>
#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

Mat cameraMatrix, distCoeffs;

void loadCalibrationData(char *filename)
{
	// --------------- Load calibration data
	FileStorage fs(filename, FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cout << "Could not open the camera calibration file..." << endl;
        return;
    }
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    distCoeffs = Mat::zeros(8, 1, CV_64F);
    fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distCoeffs;
    fs.release(); 
}

/** Identifier of the camera representing the world coordinate system. */
#define CAMID_WORLD -1

/** Represents a 2D ray in an image */
class Ray2D
{
public:
	/** ID of the camera this ray is represented for. */
	int cameraID;

	/** Starting point */
	Point2f A;

	/** Next point (direction vector is given by B-A) */
	Point2f B;

	void show(char *msg)
	{
		cout << msg << ": Ray2d(A="<<A.x<<"/"<<A.y<<"B="<<B.x<<"/"<<B.y<<")"<<endl;
	}
};

/** Representas a ray in 3D.
	This may have been an image point in one of the cameras,
	now represented in the 3D world coordinate system as a ray.
*/
class Ray
{
public:
	/** ID of the camera the ray is represented in.
		CAMID_WORLD indicates world coordinates.
		Warning: later if a camera gets a ray with with its own ID,
			transformation can be omitted only if the camera is stationary!
	*/
	int cameraID;

	/** Homog. coordinates of the origin position (like the camera focal point) (4x1 CV_64F matrix) */
	Matx41f A;
	/** Coordinates of the next point (orientation vector is given by B-A) */
	Matx41f B;
	
	/** ID of the source camera */
	int originalCameraID;
	/** Original image coordinates in the original cameras image coordinate system */
	Point2f originalImageLocation;

	void show(char *msg)
	{
		cout << msg << ": Ray("<<
			"A=("<<A.val[0]<<" "<<A.val[1]<<" "<<A.val[2]<<" "<<A.val[3]<<
			") B=("<<B.val[0]<<" "<<B.val[1]<<" "<<B.val[2]<<" "<<B.val[3]<<
			"), CamID="<<cameraID<<", origCamID="<<originalCameraID<<")"<<endl;
	}

};

// Represents a camera
class Camera
{

	Matx33f cameraMatrix;
	Mat distortionCoeffs;
	// Camera parameters extracted from cameraMatrix for faster access.
	double cx, cy, fx, fy;
public:
	/** True if the camera is stationary (R and T do not change over time) */
	bool isStationary;

	/** ID of the camera (the coordinate system) */
	int cameraID;

	/** Transformation from camera coordinate system to world coordinate system (cam->world) */
	Matx44f T;

	// Setters
	void setCameraMatrix(Mat camMtx)
	{
		CV_Assert(camMtx.rows==3);
		CV_Assert(camMtx.cols==3);
		CV_Assert(camMtx.type() == CV_64F);

		cameraMatrix = camMtx;
		// Extract camera parameters for faster access
		fx = camMtx.at<double>(0,0);
		fy = camMtx.at<double>(1,1);
		cx = camMtx.at<double>(0,2);
		cy = camMtx.at<double>(1,2);
	}

	void setDistortionCoeffs(Mat distortCoeffMtx)
	{
		CV_Assert(distortCoeffMtx.rows==8);
		CV_Assert(distortCoeffMtx.cols==1);
		CV_Assert(distortCoeffMtx.type() == CV_64F);
	
		distortionCoeffs = distortCoeffMtx;
	}

	/** Point: image -> cam (results ray!) */
	Ray pointImg2Cam(Point2f pImg)
	{
		// Orientation vector in the cameras own coordinate system
		double vx = pImg.x - cx;
		double vy = pImg.y - cy;
		double vz = fx;	// Warning! Aspect ratio is assumed to be 1 !!!

		// Create ray
		Ray ray;
		ray.originalCameraID = cameraID;
		ray.originalImageLocation = pImg;
		ray.A = Matx41f(0,0,0,1);
		ray.B = Matx41f(vx,vy,vz,1);
		ray.cameraID = cameraID;

		return ray;
	}

	/** Point: cam -> image */
	Point2f pointCam2Img(Matx41f pCam)
	{
		// Get 3D coordinates (from homogeneous coordinates)
		double x = pCam.val[0] / pCam.val[3];
		double y = pCam.val[1] / pCam.val[3];
		double z = pCam.val[2] / pCam.val[3];

		// Camera transformation
		Point2f pImg;
		pImg.x = fx * ( x / z ) + cx;
		pImg.y = fy * ( y / z ) + cy;

		return pImg;
	}

	/** Point: cam -> world */
	Matx41f pointCam2World(Matx41f pCam)
	{
		Matx41f pWorld = T * pCam;
		return pWorld;
	}

	/** Point: world -> cam */
	Matx41f pointWorld2Cam(Matx41f pWorld)
	{
		Matx41f pCam = T.inv() * pWorld;
		return pCam;
	}

	/** Ray: cam -> image */
	Ray2D rayCam2Img(Ray rCam)
	{
		CV_Assert(rCam.cameraID == cameraID);

		Ray2D rImg;
		rImg.cameraID = cameraID;
		rImg.A = pointCam2Img(rCam.A);
		rImg.B = pointCam2Img(rCam.B);

		return rImg;
	}

	/** Ray: cam -> image
		Warning: works only for rays created by the same, stationary camera! */
	Point2f rayOrigCam2Img(Ray rOrigCam)
	{
		CV_Assert(rOrigCam.originalCameraID == cameraID);
		// TODO: should be able to throw error in release mode as well!
		Point2f pImg;
		pImg = rOrigCam.originalImageLocation;
		return pImg;
	}

	/** Ray: cam -> world */
	Ray rayCam2World(Ray rCam)
	{
		CV_Assert(rCam.cameraID == cameraID);

		// Create ray
		Ray rWorld = rCam;
		rWorld.A = T * rCam.A;
		rWorld.B = T * rCam.B;
		rWorld.cameraID = CAMID_WORLD;
		return rWorld;
	}

	/** Ray: world -> cam */
	Ray rayWorld2Cam(Ray rWorld)
	{
		// Create ray
		Ray rCam = rWorld;
		rCam.A = T.inv() * rWorld.A;
		rCam.B = T.inv() * rWorld.B;
		rCam.cameraID = cameraID;
		return rCam;
	}

	/** Point image -> world (results ray) (wrapper function) */
	Ray pointImg2World(Point2f pImg)
	{
		Ray rCam = pointImg2Cam(pImg);
		Ray rWorld = rayCam2World(rCam);
		return rWorld;
	}

	/** Point world -> image (wrapper function) */
	Point2f pointWorld2Img(Mat pWorld)
	{
		Matx41f pCam = pointWorld2Cam(pWorld);
		Point2f pImg = pointCam2Img(pCam);
		return pImg;
	}

};

void show(char *msg, Matx41f value)
{
	cout << msg << ": ";
	for(int i=0; i<4; i++)
	{
		cout << value.val[i] << " ";
	}
	cout << endl;
}

void main()
{
	// Mat M = (Mat_<double>(3,3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);

/*	loadCalibrationData("test1.xml");
	for (int r=0; r<distCoeffs.rows; r++)
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

	Camera camW;
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
	cout << "p1I: " << p1I.x << " " << p1I.y << endl;	// Should be left and twice more up...

	// Testing Point Img -> Ray
	r1W = camW.pointImg2Cam(p1I);
	r1W.show("r1W (should be from origo towards 20 -10 200");




	cout << endl;
}
