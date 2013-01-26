#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>	// For marker data export into file...

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "myconfigmanager.h"
#include "detectioncollector.h"

//#include "VideoInputGeneric.h"
#include "VideoInputPs3Eye.h"

#include "chessboarddetector.h"
#include "showhelper.h"

#include "camera.h"
#include "ray.h"
#include "ray2d.h"

#include "MarkerCC2Tracker.h"
#include "TimeMeasurementCodeDefines.h"

using namespace cv;
using namespace std;
using namespace TwoColorCircleMarker;
using namespace MiscTimeAndConfig;

MyConfigManager configManager;
char *configfilename = "m1test.ini";

const char* wndCam0 = "CAM 0";
const char* wndCam1 = "CAM 1";

uchar lastR, lastG, lastB;	// Color at last click...

void mouse_callback(int eventtype, int x, int y, int flags, void *param)
{
	// param points to the corresponding matrix
	Mat *matPtr = (Mat*)param;

	if (eventtype == CV_EVENT_LBUTTONDOWN)
	{
		if (matPtr != NULL)
		{
			Vec3b result = matPtr->at<Vec3b>(y,x);
			int r = (int)(result[2] >> 5 << 5);
			int g = (int)(result[1] >> 5 << 5);
			int b = (int)(result[0] >> 5 << 5);
			cout << "Click at " << x << "-" << y << " R=" << r << " G=" << g << " B=" << b << endl;
			lastR = r;
			lastG = g;
			lastB = b;
		}
		else
		{
			cout << "Click. Error: pointer to image not set..." << endl;
		}
	}
}



void doCalibration(Camera& cam, ChessboardDetector& detector, Mat& frame)
{
	if (cam.isStationary && cam.getIsTSet())
	{
		return;
	}

	if (detector.findChessboardInFrame(frame))
	{
		drawChessboardCorners(frame,Size(9,6),detector.pointBuf,true);
		cam.calculateExtrinsicParams(detector.chessboard.corners,detector.pointBuf);
		char txt[50];
		Matx44f T = cam.GetT();

		// Show calibration data on the frame
		for(int i=0; i<16; i++)
		{
			sprintf(txt, "%4.2lf",T.val[i] );
			putText( frame, string(txt), cvPoint( 25+(i%4)*75, 20+(i/4)*20 ), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,0) );
		}
/*		if (addRays)
		{
			addRayFromCameraToOrigin(cam);
		}
		showRaysOnImage(cam,frame); */
	}
}

void doTrackingOnFrame(Camera& cam, Mat& frame, MarkerCC2Tracker& tracker, int frameIdx)
{
	// Find marker direction in both camera images and add rays corresponding these
	tracker.processFrame(frame,cam.cameraID,frameIdx);

	// Calculate marker location (if both cameras see it) and add 3D point into scene


	// Draw rays and 3D point on both frames


}

typedef enum { gain, exposure } CamParamEnum;
int getCamParamID(CamParamEnum enumValue)
{
	return (enumValue == gain ? CLEYE_GAIN : CLEYE_EXPOSURE);
}

string getCamParamName(CamParamEnum enumValue)
{
	return (enumValue == gain ? string("GAIN") : string("EXPOSURE"));
}


/** Implementation of M1 scenario
	Two stationary cameras are calibrated using a chessboard, and then
	a moving MCC2 type marker is tracked using the two cameras.
	Marker location and view rays from the cameras are visualized in both
	images (images of the two cameras).

	Cameras are not stationary initally to allow user to move them so they can
	recognize the chessboard. After that, user can set cameras to be stationary.
*/
void main()
{
	typedef enum { calibration, tracking, exiting } ModeEnum;

	// Setup config management
	configManager.init(configfilename);

	VideoInputPs3Eye videoInput0;
	videoInput0.init(0);
	VideoInputPs3Eye videoInput1;
	videoInput1.init(1);

	Mat frame0Captured(480,640,CV_8UC4);
	Mat frame1Captured(480,640,CV_8UC4);
	Mat frame0Undistorted(480,640,CV_8UC4);
	Mat frame1Undistorted(480,640,CV_8UC4);
	Mat frame0(480,640,CV_8UC3);
	Mat frame1(480,640,CV_8UC3);
	char key;
	int tmpI;
	
	// Chessboard detection and camera init
	ChessboardDetector detector(Size(9,6),50);
	Camera cam0;
	Camera cam1;
	cam0.cameraID=0;
	cam1.cameraID=1;
	cam0.isStationary = false;
	cam1.isStationary = false;
	cam0.loadCalibrationData("test1.xml");
	cam1.loadCalibrationData("test1.xml");

	// Setup marker processing
	const Size dsize(640,480);	// TODO: should always correspond to the real frame size!
	DetectionCollector detectionCollector;

	TwoColorCircleMarker::MarkerCC2Tracker tracker0;
	tracker0.setResultExporter(&detectionCollector);
	tracker0.init(configfilename,true,dsize.width,dsize.height);
	TwoColorCircleMarker::MarkerCC2Tracker tracker1;
	tracker1.setResultExporter(&detectionCollector);
	tracker1.init(configfilename,true,dsize.width,dsize.height);

	// Setup time management
	MiscTimeAndConfig::TimeMeasurement timeMeasurement;
	timeMeasurement.init();
	TimeMeasurementCodeDefs::setnames(&timeMeasurement);

	// Show hints for user
	cout << "Keys:" << endl << "(ESC) exit" << endl << "(s) Cameras are stationary" << endl;
	cout << "(m) Cameras are moving" << endl << "(t) Tracking mode" << endl << "(c) back to calibration mode" << endl;
	cout << "(0) and (1) adjust camera parameters for camera 0 and 1." << endl;
	cout << "Adjust (g) gain or (e) exposure with (+) and (-)" << endl;

	// Setup windows and mouse callback
	namedWindow(wndCam0, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(wndCam0, mouse_callback, (void*)&frame0);
	namedWindow(wndCam1, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(wndCam1, mouse_callback, (void*)&frame1);

	// Start main loop
	int adjustCam = 0;
	CamParamEnum camParam = exposure;
	int frameIdx = 0;
	ModeEnum mode = calibration;
	while(mode != exiting)
	{
		detectionCollector.currentFrameIdx = frameIdx;
		videoInput0.captureFrame(frame0Captured);
		videoInput1.captureFrame(frame1Captured);

		cam0.undistortImage(frame0Captured,frame0Undistorted);
		cam1.undistortImage(frame1Captured,frame1Undistorted);

		// Convert frames from CV_8UC4 to CV_8UC3
		cvtColor(frame0Undistorted,frame0,CV_BGRA2BGR);
		cvtColor(frame1Undistorted,frame1,CV_BGRA2BGR);
		
		// During calibration, search for chessboard and run calibration if it was found.
		if (mode == calibration)
		{
			doCalibration(cam0,detector,frame0);
			doCalibration(cam1,detector,frame1);
		}

		// During tracking, search for marker and calculate location info
		if (mode == tracking)
		{
			// Remove rays of previous frames
			//detectionCollector.startNewFrame();

			// Track marker on both frames
			detectionCollector.cam = &cam0;
			doTrackingOnFrame(cam0,frame0,tracker0,frameIdx);
			detectionCollector.cam = &cam1;
			doTrackingOnFrame(cam1,frame1,tracker1,frameIdx);
			
			// Display rays in both cameras
			detectionCollector.ShowRaysInFrame(frame0,cam0);
			detectionCollector.ShowRaysInFrame(frame1,cam1);
		}

		imshow("CAM 0",frame0);
		imshow("CAM 1",frame1);
		imshow("CAM 0 CC",*(tracker0.visColorCodeFrame));
		imshow("CAM 1 CC",*(tracker1.visColorCodeFrame));

		key = waitKey(25);
		switch(key)
		{
		case 27:	// ESC: exit
			mode = exiting;
			break;
		case 's':	// Set cameras stationary
			cam0.isStationary = true;
			cam1.isStationary = true;
			cout << "Cameras are now STATIONARY." << endl;
			break;
		case 'm':	// Set cameras moving (not stationary)
			cam0.isStationary = false;
			cam1.isStationary = false;
			cout << "Cameras are now MOVING (not stationary)." << endl;
			break;
		case 'c':	// Calibration mode
			mode = calibration;
			cout << "Now in CALIBRATION mode." << endl;
			break;
		case 't':	// Tracking mode
			mode = tracking;
			cout << "Now in TRACKING mode." << endl;
			break;
		case '0':	// Adjust cam 0
			adjustCam = 0;
			cout << "Now adjusting " << getCamParamName(camParam) << " of cam " << adjustCam << endl;
			break;
		case '1':	// Adjust cam 1
			adjustCam = 1;
			cout << "Now adjusting " << getCamParamName(camParam) << " of cam " << adjustCam << endl;
			break;
		case 'e':	// Adjust exposure
			camParam = exposure;
			cout << "Now adjusting " << getCamParamName(camParam) << " of cam " << adjustCam << endl;
			break;
		case 'g':	// Adjust gain
			camParam = gain;
			cout << "Now adjusting " << getCamParamName(camParam) << " of cam " << adjustCam << endl;
			break;
		case '+':	// Increase adjusted parameter
			if (adjustCam==0)
			{
				tmpI = videoInput0.IncrementCameraParameter(getCamParamID(camParam));
			}
			else
			{
				tmpI = videoInput1.IncrementCameraParameter(getCamParamID(camParam));
			}
			cout << "Increased " << getCamParamName(camParam) << " or camera " << adjustCam << " to " << tmpI << endl;
			break;
		case '-':	// Decreases adjusted parameter
			if (adjustCam==0)
			{
				tmpI = videoInput0.DecrementCameraParameter(getCamParamID(camParam));
			}
			else
			{
				tmpI = videoInput1.DecrementCameraParameter(getCamParamID(camParam));
			}
			cout << "Decreased " << getCamParamName(camParam) << " or camera " << adjustCam << " to " << tmpI << endl;
			break;
		case 'r':	// Last clicked color should be red
			if (adjustCam==0)
			{
				tracker0.fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_RED);
			}
			else
			{
				tracker1.fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_RED);
			}
			cout << "Color LUT updated for RED in tracker for cam " << adjustCam << "for color R"<<lastR<<"G"<<lastG<<"B"<<lastB << endl;
			break;
		case 'b':	// Last clicked color should be blue
			if (adjustCam==0)
			{
				tracker0.fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_BLU);
			}
			else
			{
				tracker1.fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_BLU);
			}
			cout << "Color LUT updated for BLU in tracker for cam " << adjustCam << "for color R"<<lastR<<"G"<<lastG<<"B"<<lastB << endl;
			break;

		}

		frameIdx++;
	}
	videoInput0.release();
	videoInput1.release();
}
