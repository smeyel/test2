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
	/** High precision time measurement tool.
		Time measurement is performed using platform specific performance counters
			provided by OpenCV in a platform independent way.
	*/
	class TimeMeasurement
	{
		int64 currentStartValues[MAX_TIMING_CODE];
		int64 sumvalues[MAX_TIMING_CODE];
		int64 numvalues[MAX_TIMING_CODE];
		string names[MAX_TIMING_CODE];
		string measurementName;	// Used as a header before the measurement

		double tickFrequency;

	public:
		/** Initialize the time measurements.
		*/
		void init();

		/** Set the name of the measurement. It will be displayed by
				showresults().
		*/
		void setMeasurementName(string name);

		/** Set the description for a given measurementID. During preparation,
				call this for every valid measurementID to give it a name.
		*/
		void setname(int measurementid, string name);

		/** Call this before starting a given operation identified my measurementID.
			Multiple start-finish call pairs are allowed, the average of the elapsed
			times will be calculated.
		*/
		void start(int measurementid);

		/** Call this after finishing an overation.
		*/
		double finish(int measurementid);

		/** Get average elapsed time in ms for a given measurementID.
		*/
		double getavgms(int measurementid);

		/** By defining a measurementID representing the whole processing
				of a frame, the maximal achievable FPS is returned.
		*/
		double getmaxfps(int measurementid);

		/** Displays the measurement results on the standard output.
		*/
		void showresults();

		/** Writes the measurement results on the given output stream.
		*/
		void showresults(std::ostream *stream);
	};
}

#endif
