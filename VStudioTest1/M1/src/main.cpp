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
char *configfilename = "../testini.ini";


// -----------------------------------------------------------

void doCalibration(Camera& cam, ChessboardDetector& detector, Mat& frame)
{
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



/** Implementation of M1 scenario
	Two stationary cameras are calibrated using a chessboard, and then
	a moving MCC2 type marker is tracked using the two cameras.
	Marker location and view rays from the cameras are visualized in both
	images (images of the two cameras).

	Cameras are not stationary initally to allow user to move them so they can
	recognize the chessboard. After that, user can set cameras to be stationary.
*/
void test_rayshow()
{
	typedef enum { calibration, tracking, exiting } ModeEnum;

	// Setup config management
	configManager.init(configfilename);

	VideoInputPs3Eye videoInput0;
	videoInput0.init(0);
	VideoInputPs3Eye videoInput1;
	videoInput1.init(1);

	Mat frame0(480,640,CV_8UC4);
	Mat frame1(480,640,CV_8UC4);
	Mat frameUndistort0(480,640,CV_8UC4);
	Mat frameUndistort1(480,640,CV_8UC4);
	char key;

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

	// Start main loop
	int frameIdx = 0;
	ModeEnum mode = calibration;
	while(mode != exiting)
	{
		videoInput0.captureFrame(frame0);
		videoInput1.captureFrame(frame1);

		cam0.undistortImage(frame0,frameUndistort0);
		cam1.undistortImage(frame1,frameUndistort1);

		// During calibration, search for chessboard and run calibration if it was found.
		if (mode == calibration)
		{
			doCalibration(cam0,detector,frameUndistort0);
			doCalibration(cam1,detector,frameUndistort1);
		}

		// During tracking, search for marker and calculate location info
		if (mode == tracking)
		{
			doTrackingOnFrame(cam0,frameUndistort0,tracker0,frameIdx);
			doTrackingOnFrame(cam1,frameUndistort1,tracker1,frameIdx);
		}

		imshow("CAM 0",frameUndistort0);
		imshow("CAM 1",frameUndistort1);
		imshow("CAM 0 CC",*(tracker0.colorCodeFrame));
		imshow("CAM 1 CC",*(tracker1.colorCodeFrame));

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
		}

		frameIdx++;
	}
	videoInput0.release();
	videoInput1.release();
}


void main()
{
	test_rayshow();
}
