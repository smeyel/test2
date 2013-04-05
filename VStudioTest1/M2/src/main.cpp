#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>	// For marker data export into file...

#include <windows.h>	// for sleep

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../include/PhoneProxy.h"

#include "myconfigmanager.h"

//#include "VideoInputFactory.h"
//#include "VideoInputPs3EyeParameters.h"

#include "../include/TimeMeasurementCodeDefines.h"

using namespace cv;
using namespace std;
using namespace MiscTimeAndConfig;

MyConfigManager configManager;
char *configfilename = "m2_default.ini";

/** Implementation of M2 scenario
*/
void main()
{
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

	// Setup time management
	MiscTimeAndConfig::TimeMeasurement timeMeasurement;
	timeMeasurement.init();
	M2::TimeMeasurementCodeDefs::setnames(&timeMeasurement);

	timeMeasurement.start(M2::TimeMeasurementCodeDefs::FullExecution);
	bool running=true;
	int frameIdx = 0;
	while(running)
	{
		timeMeasurement.start(M2::TimeMeasurementCodeDefs::FrameAll);

		// Request image from the phone

		char *ip = "152.66.173.64";
		int port = 6000;
		PhoneProxy proxy;

		char tmpBuff[100];
		/*cout << "Press enter to start connecting..." << endl;
		cin >> tmpBuff;*/

/*		proxy.Connect(ip,port);
		proxy.RequestPing();
		proxy.ReceiveDebug();
		//proxy.Receive("d:\\temp\\nothing.txt");
		proxy.Disconnect();*/

		for(int i=0; i<10; i++)
		{
			cout << "Capture No " << i << "..." << endl;
			cout << "Connecting..." << endl;
			proxy.Connect(ip,port);
			cout << "Requesing photo..." << endl;
			timeMeasurement.start(M2::TimeMeasurementCodeDefs::Send);
			proxy.RequestPhoto(0);
			timeMeasurement.finish(M2::TimeMeasurementCodeDefs::Send);
			cout << "Receiving photo..." << endl;
			timeMeasurement.start(M2::TimeMeasurementCodeDefs::WaitAndReceive);
			proxy.Receive("d:\\temp\\image1.jpg");
			//proxy.ReceiveDebug();
			timeMeasurement.finish(M2::TimeMeasurementCodeDefs::WaitAndReceive);
			cout << "Disconnecting..." << endl;
			proxy.Disconnect();
			Sleep(3000);
		}

		/*cout << "Connecting..." << endl;
		proxy.Connect(ip,port);
		cout << "Requesing photo..." << endl;
		timeMeasurement.start(M2::TimeMeasurementCodeDefs::Send);
		proxy.RequestPhoto(0);
		timeMeasurement.finish(M2::TimeMeasurementCodeDefs::Send);
		cout << "Receiving photo..." << endl;
		timeMeasurement.start(M2::TimeMeasurementCodeDefs::WaitAndReceive);
		proxy.Receive("d:\\temp\\image2.jpg");
		timeMeasurement.finish(M2::TimeMeasurementCodeDefs::WaitAndReceive);
		cout << "Disconnecting..." << endl;
		proxy.Disconnect();*/

		cout << "Done..." << endl;

		running = false;

		timeMeasurement.finish(M2::TimeMeasurementCodeDefs::FrameAll);
		frameIdx++;
	}
	timeMeasurement.finish(M2::TimeMeasurementCodeDefs::FullExecution);
	log << "--- Main loop time measurement results:" << endl;
	timeMeasurement.showresults(&log);

	log << "--- Further details:" << endl;
	log << "max fps: " << timeMeasurement.getmaxfps(M2::TimeMeasurementCodeDefs::FrameAll) << endl;
	log << "Number of processed frames: " << frameIdx << endl;

	log.flush();
	log.close();

	cout << "Done." << endl;
}
