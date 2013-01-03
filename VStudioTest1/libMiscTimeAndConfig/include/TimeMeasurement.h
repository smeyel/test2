#ifndef __TIMEMEASUREMENT_H_
#define __TIMEMEASUREMENT_H_

#include <assert.h>
#include <time.h>
#include <iostream>
#include <string>

#define MAX_TIMING_CODE 100

using namespace std;

namespace MiscTimeAndConfig
{
	class TimeMeasurement
	{
		int64 currentStartValues[MAX_TIMING_CODE];
		int64 sumvalues[MAX_TIMING_CODE];
		int64 numvalues[MAX_TIMING_CODE];
		string names[MAX_TIMING_CODE];
		string measurementName;	// Used as a header before the measurement

		double tickFrequency;

	public:
		void init();
		void setMeasurementName(string name);
		void setname(int measurementid, string name);
		void start(int measurementid);
		double finish(int measurementid);
		double getavgms(int measurementid);
		double getmaxfps(int measurementid);
		void showresults();
		void showresults(std::ostream *stream);
	};
}

#endif
