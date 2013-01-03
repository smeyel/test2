#include <opencv2/core/core.hpp>

#include "TimeMeasurement.h"

#define USEPRECISIONTIMING	// Use getTickCount() instead of clock()

using namespace MiscTimeAndConfig;

void TimeMeasurement::init()
{
	for(int i=0; i<MAX_TIMING_CODE; i++)
	{
		currentStartValues[i]=0;
		sumvalues[i]=0;
		numvalues[i]=0;
	}
	tickFrequency = cv::getTickFrequency();
}

void TimeMeasurement::setname(int measurementid, string name)
{
	assert(measurementid>=0);
	assert(measurementid<MAX_TIMING_CODE);
	names[measurementid]=name;
}

void TimeMeasurement::setMeasurementName(string name)
{
	measurementName = name;
}

void TimeMeasurement::start(int measurementid)
{
	assert(measurementid<MAX_TIMING_CODE);
#ifdef USEPRECISIONTIMING
	currentStartValues[measurementid] = cv::getTickCount();
#else
	currentStartValues[measurementid] = clock();
#endif
}

double TimeMeasurement::finish(int measurementid)
{
	assert(measurementid<MAX_TIMING_CODE);
	int64 currentTime;
#ifdef USEPRECISIONTIMING
	currentTime = cv::getTickCount();
#else
	currentTime = clock();
#endif

	long current = currentTime - currentStartValues[measurementid];
	sumvalues[measurementid] += current;
	numvalues[measurementid]++;
	// Returns in ms
	return (double)current / tickFrequency * 1000.0;
}

double TimeMeasurement::getavgms(int measurementid)
{
	assert(measurementid<MAX_TIMING_CODE);
	if (numvalues[measurementid]==0)
	{
		return 0.0;	// Avoiding division by zero
	}
	double avgMs;
#ifdef USEPRECISIONTIMING
	// tick frequency is given in Hz
	avgMs = (double)sumvalues[measurementid] / tickFrequency / (double)numvalues[measurementid] * 1000.0;
#else
	// clock() uses milliseconds
	avgMs = (double)sumvalues[measurementid] / (double)numvalues[measurementid];
#endif
	return avgMs;
}

double TimeMeasurement::getmaxfps(int measurementid)
{
	return 1000.0/getavgms(measurementid);
}

void TimeMeasurement::showresults()
{
	showresults(&cout);
}

void TimeMeasurement::showresults(std::ostream *stream)
{
	*stream << "Time measurements for " << measurementName << ":" << endl;
	*stream << "(Average execution times (using tick frequency " << tickFrequency << " Hz)" << endl;
	for (int i=0; i<MAX_TIMING_CODE; i++)
	{
		float avg = getavgms(i);
		if (avg != 0.0)	// not the default value (in this case, can be checked with ==)
		{
			*stream << names[i] << ": " << avg << " ms" << endl;
		}
	}

}
