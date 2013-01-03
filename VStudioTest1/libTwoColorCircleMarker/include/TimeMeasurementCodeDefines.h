#ifndef __TIMEMEASUREMENTCODEDEFINES_H_
#define __TIMEMEASUREMENTCODEDEFINES_H_

#include "TimeMeasurement.h"
using namespace MiscTimeAndConfig;

namespace TwoColorCircleMarker
{
	class TimeMeasurementCodeDefs
	{
	public:
		const static int ProcessAll			= 0;	// Processing a frame
		const static int FastColorFilter	= 1;	// 
		const static int VisualizeDecomposedImage = 2;	// 
		const static int TwoColorLocator	= 3;	// 
		const static int LocateMarkers		= 4;	// 

		static void setnames(TimeMeasurement *measurement)
		{
			measurement->setMeasurementName("TwoColorCircleMarker");

			measurement->setname(ProcessAll,"ProcessAll");
			measurement->setname(FastColorFilter,"FastColorFilter");
			measurement->setname(VisualizeDecomposedImage,"VisualizeDecomposedImage");
			measurement->setname(TwoColorLocator,"TwoColorLocator");
			measurement->setname(LocateMarkers,"LocateMarkers");
		}
	};
}

#endif