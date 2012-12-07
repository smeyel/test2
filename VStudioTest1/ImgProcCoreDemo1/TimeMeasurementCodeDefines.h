#ifndef __TIMEMEASUREMENTCODEDEFINES_H_
#define __TIMEMEASUREMENTCODEDEFINES_H_

#include "TimeMeasurement.h"

class TimeMeasurementCodeDefs
{
public:
	const static int FrameAll			= 0;	// Processing a frame
	const static int Capture			= 1;	// Capturing a frame from the input source
	const static int Resize				= 2;	// Capturing a frame from the input source
	const static int TwoColorLocator	= 3;	// 
	const static int  FastColorFilter	= 4;	// 
	const static int  IntegralImages	= 5;	// 
	const static int  GetCandidateRectangles	= 6;	// 
	const static int  ProcessIntegralImages		= 7;	// 
	const static int LocateMarkers		= 8;	// 
	const static int  MarkerScanlines	= 9;	// 
	const static int  MarkerFitEllipses	= 10;	// 
	const static int  MarkerScanEllipses	= 11;	// 
	const static int  ConsolidateValidate	= 12;	// 
	const static int ShowImages			= 13;	// 

	static void setnames()
	{
		TimeMeasurement::instance.setname(FrameAll,"FrameAll");
		TimeMeasurement::instance.setname(Capture,"Capture");
		TimeMeasurement::instance.setname(Resize,"Resize");
		TimeMeasurement::instance.setname(TwoColorLocator,"TwoColorLocator");
		TimeMeasurement::instance.setname(LocateMarkers,"LocateMarkers");
		TimeMeasurement::instance.setname(MarkerScanlines,"LocateMarkers.MarkerScanlines");
		TimeMeasurement::instance.setname(MarkerFitEllipses,"LocateMarkers.MarkerFitEllipses");
		TimeMeasurement::instance.setname(ConsolidateValidate,"LocateMarkers.ConsolidateValidate");
		TimeMeasurement::instance.setname(ShowImages,"ShowImages");
		TimeMeasurement::instance.setname(IntegralImages,"TwoColorLocator.IntegralImages");
		TimeMeasurement::instance.setname(GetCandidateRectangles,"TwoColorLocator.GetCandidateRectangles");
		TimeMeasurement::instance.setname(ProcessIntegralImages,"TwoColorLocator.ProcessIntegralImages");
		TimeMeasurement::instance.setname(FastColorFilter,"FastColorFilter");
	}
};

#endif