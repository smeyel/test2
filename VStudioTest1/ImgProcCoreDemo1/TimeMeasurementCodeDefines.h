#ifndef __TIMEMEASUREMENTCODEDEFINES_H_
#define __TIMEMEASUREMENTCODEDEFINES_H_

#include "TimeMeasurement.h"
using namespace MiscTimeAndConfig;

class TimeMeasurementCodeDefs
{
public:
	const static int FrameAll			= 0;	// Processing a frame
	const static int Capture			= 1;	// Capturing a frame from the input source
	const static int Resize				= 2;	// Capturing a frame from the input source
	const static int Process			= 3;	// 
	const static int ShowImages			= 4;	// 
	const static int InterFrameDelay	= 5;	// 
	const static int FullExecution		= 6;	// 

	static void setnames(TimeMeasurement *measurement)
	{
		measurement->setMeasurementName("Main loop");

		measurement->setname(FrameAll,"FrameAll");
		measurement->setname(Capture,"Capture");
		measurement->setname(Resize,"Resize");
		measurement->setname(Process,"Process");
		measurement->setname(ShowImages,"ShowImages");
		measurement->setname(InterFrameDelay,"InterFrameDelay");
		measurement->setname(FullExecution,"FullExecution");
	}
};

#endif