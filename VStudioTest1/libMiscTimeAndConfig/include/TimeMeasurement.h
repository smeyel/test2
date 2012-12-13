#ifndef __TIMEMEASUREMENT_H_
#define __TIMEMEASUREMENT_H_

#include <assert.h>
#include <time.h>
#include <iostream>
#include <string>

#define MAX_TIMING_CODE 100

using namespace std;

class TimeMeasurement
{
	clock_t currentStartValues[MAX_TIMING_CODE];
	long sumvalues[MAX_TIMING_CODE];
	int numvalues[MAX_TIMING_CODE];
	string names[MAX_TIMING_CODE];

public:

	static TimeMeasurement instance;

	void init();
	void setname(int measurementid, string name);
	void start(int measurementid);
	long finish(int measurementid);
	float getavgms(int measurementid);
	float getmaxfps(int measurementid);
	void showresults();
};

#endif
