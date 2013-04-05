#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>	// For marker data export into file...

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

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
	M1::TimeMeasurementCodeDefs::setnames(&timeMeasurement);

	timeMeasurement.start(M1::TimeMeasurementCodeDefs::FullExecution);
	bool running=true;
	int frameIdx = 0;
	while(running)
	{
		timeMeasurement.start(M1::TimeMeasurementCodeDefs::FrameAll);

		// Request image from the phone


		running = false;

		timeMeasurement.finish(M1::TimeMeasurementCodeDefs::FrameAll);
		frameIdx++;
	}
	timeMeasurement.finish(M1::TimeMeasurementCodeDefs::FullExecution);
	log << "--- Main loop time measurement results:" << endl;
	timeMeasurement.showresults(&log);

	log << "--- Further details:" << endl;
	log << "max fps: " << timeMeasurement.getmaxfps(M1::TimeMeasurementCodeDefs::FrameAll) << endl;
	log << "Number of processed frames: " << frameIdx << endl;

	log.flush();
	log.close();

	cout << "Done." << endl;
}
