#include "TimeMeasurement.h"

TimeMeasurement TimeMeasurement::instance;

void TimeMeasurement::init()
{
	for(int i=0; i<MAX_TIMING_CODE; i++)
	{
		currentStartValues[i]=0;
		sumvalues[i]=0;
		numvalues[i]=0;
	}
}

void TimeMeasurement::setname(int measurementid, string name)
{
	assert(measurementid<MAX_TIMING_CODE);
	names[measurementid]=name;
}

void TimeMeasurement::start(int measurementid)
{
	assert(measurementid<MAX_TIMING_CODE);
	currentStartValues[measurementid] = clock();
}

long TimeMeasurement::finish(int measurementid)
{
	assert(measurementid<MAX_TIMING_CODE);
	long current = clock() - currentStartValues[measurementid];
	sumvalues[measurementid] += current;
	numvalues[measurementid]++;
	return current;
}

float TimeMeasurement::getavgms(int measurementid)
{
	assert(measurementid<MAX_TIMING_CODE);
	if (numvalues[measurementid]==0)
	{
		return 0.0;	// Avoiding division by zero
	}
	float avg = (float)sumvalues[measurementid] / (float)numvalues[measurementid];
	return avg;
}

float TimeMeasurement::getmaxfps(int measurementid)
{
	return 1000.0F/getavgms(measurementid);
}

void TimeMeasurement::showresults()
{
	for (int i=0; i<MAX_TIMING_CODE; i++)
	{
		float avg = getavgms(i);
		if (avg != 0.0)	// not the default value (in this case, can be checked with ==)
		{
			cout << "Avg time for " << names[i] << " is " << avg << " ms" << endl;
		}
	}

}
