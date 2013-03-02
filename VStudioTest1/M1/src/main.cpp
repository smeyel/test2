#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>	// For marker data export into file...

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "myconfigmanager.h"
#include "detectioncollector.h"

#include "VideoInputFactory.h"
#include "VideoInputPs3EyeParameters.h"

#include "chessboarddetector.h"
#include "showhelper.h"

#include "camera.h"
#include "ray.h"
#include "ray2d.h"

#include "MarkerCC2Tracker.h"
#include "../include/TimeMeasurementCodeDefines.h"

using namespace cv;
using namespace std;
using namespace TwoColorCircleMarker;
using namespace MiscTimeAndConfig;

MyConfigManager configManager;
//char *configfilename = "m1_interactive_record1.ini";
//char *configfilename = "m1_interactive_record2wide.ini";
char *configfilename = "m1_timing_record1.ini";
//char *configfilename = "m1_timing_record2wide.ini";

const char* wndCam0 = "CAM 0";
const char* wndCam1 = "CAM 1";
const char* wndCam2 = "CAM 2";

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
	return (enumValue == gain ? VIDEOINPUTPS3EYEPARAMETERS_GAIN : VIDEOINPUTPS3EYEPARAMETERS_EXPOSURE);
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
#define CAMNUM	3
void main()
{
	typedef enum { calibration, tracking, exiting } ModeEnum;

	// Setup config management
	configManager.init(configfilename);

	// Setup statistics output file
	ofstream log;
	log.open(configManager.logFileName,ios_base::out);
	cout << "Log is written to: " << configManager.logFileName << endl;

	// Write current time and date to log
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
	log << "Current time: " 
		<< (now->tm_year + 1900) << '-' 
        << (now->tm_mon + 1) << '-'
        <<  now->tm_mday << " "
		<< now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec
        << endl;

	// Write name of config and input files into log
	log << "Config file: " << configfilename << endl;
	log << "Output (CSV) file: " << configManager.outputFileName << endl;
	log << "Input cam files: " << endl <<
		"  " << configManager.cam0FileName << endl <<
		"  " << configManager.cam1FileName << endl <<
		"  " << configManager.cam2FileName << endl;

	VideoInput *videoInputs[CAMNUM];

	for (int i=0; i<CAMNUM; i++)
	{
		videoInputs[i] = VideoInputFactory::CreateVideoInput(VIDEOINPUTTYPE_GENERIC);
	}
	videoInputs[0]->init(configManager.cam0FileName.data());
	videoInputs[1]->init(configManager.cam1FileName.data());
	videoInputs[2]->init(configManager.cam2FileName.data());

	Mat *frameCaptured[CAMNUM];
	Mat *frameUndistorted[CAMNUM];
	Mat *frame[CAMNUM];
	for(int i=0; i<CAMNUM; i++)
	{
		frameCaptured[i] = new Mat(480,640,CV_8UC4);
		frameUndistorted[i] = new Mat(480,640,CV_8UC4);
		frame[i] = new Mat(480,640,CV_8UC3);
	}

	char key;
	int tmpI;
	
	// Chessboard detection and camera init
	ChessboardDetector detector(Size(9,6),36.1);	// Chessboard cell size is 36x36mm
	Camera *cams[CAMNUM];
	for(int i=0; i<CAMNUM; i++)
	{
		cams[i] = new Camera();
		cams[i]->cameraID=i;
		cams[i]->isStationary=false;
		cams[i]->loadCalibrationData(configManager.camIntrinsicParamsFileName.data());
	}

	// Setup marker processing
	const Size dsize(640,480);	// TODO: should always correspond to the real frame size!
	DetectionCollector detectionCollector;
	detectionCollector.open(configManager.outputFileName.data());

	TwoColorCircleMarker::MarkerCC2Tracker *trackers[CAMNUM];
	for(int i=0; i<CAMNUM; i++)
	{
		trackers[i] = new TwoColorCircleMarker::MarkerCC2Tracker();
		trackers[i]->setResultExporter(&detectionCollector);
		trackers[i]->init(configfilename,true,dsize.width,dsize.height);
	}

	// Setup time management
	MiscTimeAndConfig::TimeMeasurement timeMeasurement;
	timeMeasurement.init();
	M1::TimeMeasurementCodeDefs::setnames(&timeMeasurement);

	// Show hints for user
	cout << "Keys:" << endl << "(ESC) exit" << endl << "(s) Cameras are stationary" << endl;
	cout << "(m) Cameras are moving" << endl << "(t) Tracking mode" << endl << "(c) back to calibration mode" << endl;
	cout << "(0),(1) and (2) adjust camera parameters for camera 0, 1 and 2." << endl;
	cout << "Adjust (g) gain or (e) exposure with (+) and (-)" << endl;

	// Setup windows and mouse callback
	if (configManager.showInputImage)
	{
		namedWindow(wndCam0, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(wndCam0, mouse_callback, (void*)frameCaptured[0]);
		namedWindow(wndCam1, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(wndCam1, mouse_callback, (void*)frameCaptured[1]);
		namedWindow(wndCam2, CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback(wndCam2, mouse_callback, (void*)frameCaptured[2]);
	}

	// Start main loop
	int adjustCam = 0;
	CamParamEnum camParam = exposure;
	int frameIdx = -1;
	ModeEnum mode = calibration;
	bool videoInputRunning = true;	// If false, main loop runs without capturing new frames (useful with AVI files)
	timeMeasurement.start(M1::TimeMeasurementCodeDefs::FullExecution);
	if (!configManager.interactive)
	{
		cout << "Starting processing in non-interactive mode..." << endl;
	}

	while(mode != exiting)
	{
		timeMeasurement.start(M1::TimeMeasurementCodeDefs::FrameAll);
		if (videoInputRunning)
		{
			frameIdx++;
			const int framePerSec = 30*CAMNUM;
			if (frameIdx % (30*framePerSec) == 0)
			{
				cout << "Sec: " << (frameIdx/(30*framePerSec)) << endl;
			}
			
			detectionCollector.currentFrameIdx = frameIdx;
			timeMeasurement.start(M1::TimeMeasurementCodeDefs::Capture);
			for(int i=0; i<CAMNUM; i++)
			{
				videoInputs[i]->captureFrame(*frameCaptured[i]);
				if (frameCaptured[i]->empty())
				{
					cout << "End of video" << endl;
					mode=exiting;
					break;
				}
	
				//frameCaptured[i]->copyTo(*frameUndistorted[i]);
				cams[i]->undistortImage(*frameCaptured[i],*frameUndistorted[i]);

				// Convert frames from CV_8UC4 to CV_8UC3
				cvtColor(*frameUndistorted[i],*frame[i],CV_BGRA2BGR);
			}
			timeMeasurement.finish(M1::TimeMeasurementCodeDefs::Capture);

			// During calibration, search for chessboard and run calibration if it was found.
			if (mode == calibration)
			{
				timeMeasurement.start(M1::TimeMeasurementCodeDefs::Calibration);
				bool wasUncalibratedCam = false;
				for(int i=0; i<CAMNUM; i++)
				{
					// Reset calibration data
					cams[i]->resetT();
					// Perform calibration data
					doCalibration(*cams[i],detector,*frame[i]);
					
					if (!(cams[i]->getIsTSet()))
					{
						wasUncalibratedCam = true;
					}
				}
				if (!wasUncalibratedCam)
				{
					// All cameras calibrated using the same frame!
					// Stopping calibration...
					cout << "Simultaneous calibration complete. Switching to TRACKING mode." << endl;
					mode = tracking;
				}
				timeMeasurement.finish(M1::TimeMeasurementCodeDefs::Calibration);
			}

			// During tracking, search for marker and calculate location info
			if (mode == tracking)
			{
				// Remove rays of previous frames
				//detectionCollector.startNewFrame();

				// Track marker on both frames
				timeMeasurement.start(M1::TimeMeasurementCodeDefs::Tracking);
				for(int i=0; i<CAMNUM; i++)
				{
					detectionCollector.cam = cams[i];
					doTrackingOnFrame(*cams[i],*frame[i],*trackers[i],frameIdx);
	
					// Display rays in both cameras
					detectionCollector.ShowRaysInFrame(*frame[i],*cams[i]);
				}
				timeMeasurement.finish(M1::TimeMeasurementCodeDefs::Tracking);
			}
		}

		timeMeasurement.start(M1::TimeMeasurementCodeDefs::ShowImages);
		if (configManager.showInputImage)
		{
			imshow(wndCam0,*frame[0]);
			imshow(wndCam1,*frame[1]);
			imshow(wndCam2,*frame[2]);
		}
		if (configManager.verboseColorCodedFrame)
		{
			imshow("CAM 0 CC",*(trackers[0]->visColorCodeFrame));
			imshow("CAM 1 CC",*(trackers[1]->visColorCodeFrame));
			imshow("CAM 2 CC",*(trackers[2]->visColorCodeFrame));
		}
		timeMeasurement.finish(M1::TimeMeasurementCodeDefs::ShowImages);

		if (configManager.interactive)
		{
			timeMeasurement.start(M1::TimeMeasurementCodeDefs::InterFrameDelay);
			key = waitKey(25);
			timeMeasurement.finish(M1::TimeMeasurementCodeDefs::InterFrameDelay);
			switch(key)
			{
			case 27:	// ESC: exit
				mode = exiting;
				break;
			case 'p':	// Pause video capture
				if (videoInputRunning)
				{
					videoInputRunning = false;
					cout << "Video capture suspended. Press 'p' to resume." << endl;
				}
				else
				{
					videoInputRunning = true;
					cout << "Video capture resumed." << endl;
				}
				break;
			case 's':	// Set cameras stationary
				for(int i=0; i<CAMNUM; i++)
				{
					cams[i]->isStationary = true;
				}
				cout << "Cameras are now STATIONARY." << endl;
				break;
			case 'm':	// Set cameras moving (not stationary)
				for(int i=0; i<CAMNUM; i++)
				{
					cams[i]->isStationary = false;
				}
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
			case '2':	// Adjust cam 2
				adjustCam = 2;
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
				tmpI = videoInputs[adjustCam]->IncrementCameraParameter(getCamParamID(camParam));
				cout << "Increased " << getCamParamName(camParam) << " or camera " << adjustCam << " to " << tmpI << endl;
				break;
			case '-':	// Decreases adjusted parameter
				tmpI = videoInputs[adjustCam]->DecrementCameraParameter(getCamParamID(camParam));
				cout << "Decreased " << getCamParamName(camParam) << " or camera " << adjustCam << " to " << tmpI << endl;
				break;
			case 'r':	// Last clicked color should be red
				trackers[adjustCam]->fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_RED);
				cout << "Color LUT updated for RED in tracker for cam " << adjustCam << "for color R"<<lastR<<"G"<<lastG<<"B"<<lastB << endl;
				break;
			case 'b':	// Last clicked color should be blue
				trackers[adjustCam]->fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_BLU);
				cout << "Color LUT updated for BLU in tracker for cam " << adjustCam << "for color R"<<lastR<<"G"<<lastG<<"B"<<lastB << endl;
				break;
			case 'n':	// Last clicked color should be NONE
				trackers[adjustCam]->fastColorFilter.setLutItem(lastR,lastG,lastB,COLORCODE_NONE);
				cout << "Color LUT updated for NONE in tracker for cam " << adjustCam << "for color R"<<lastR<<"G"<<lastG<<"B"<<lastB << endl;
				break;
			}
		}
		timeMeasurement.finish(M1::TimeMeasurementCodeDefs::FrameAll);
	}
	timeMeasurement.finish(M1::TimeMeasurementCodeDefs::FullExecution);
	detectionCollector.close();
	for(int i=0; i<CAMNUM; i++)
	{
		videoInputs[i]->release();
	}

	log << "--- Main loop time measurement results:" << endl;
	timeMeasurement.showresults(&log);
	for(int i=0; i<CAMNUM; i++)
	{
		log << "--- Tracker time measurements for CAM " << i << ":" << endl;
		trackers[i]->timeMeasurement->showresults(&log);
	}

	log << "--- Further details:" << endl;
	log << "max fps: " << timeMeasurement.getmaxfps(M1::TimeMeasurementCodeDefs::FrameAll) << endl;
	log << "Number of processed frames: " << frameIdx << endl;

	log.flush();
	log.close();

	cout << "Done." << endl;
}
