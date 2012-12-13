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
	const static int FastColorFilter	= 3;	// 
	const static int TwoColorLocator	= 4;	// 
	const static int  ApplyOnCC			= 5;	// 
	const static int  IntegralImages	= 6;	// 
	const static int  GetCandidateRectangles	= 7;	// 
	const static int  ProcessIntegralImages		= 8;	// 
	const static int  ConsolidateRectangles		= 9;	// 
	const static int LocateMarkers		= 10;	// 
	const static int  MarkerScanlines	= 11;	// 
	const static int  MarkerFitEllipses	= 12;	// 
	const static int  MarkerScanEllipses	= 13;	// 
	const static int  ConsolidateValidate	= 14;	// 
	const static int ShowImages			= 15;	// 
	const static int InterFrameDelay	= 16;	// 
	const static int FullExecution		= 17;	// 

	static void setnames()
	{
		TimeMeasurement::instance.setname(FrameAll,"FrameAll");
		TimeMeasurement::instance.setname(Capture,"Capture");
		TimeMeasurement::instance.setname(Resize,"Resize");

		TimeMeasurement::instance.setname(FastColorFilter,"FastColorFilter");
		TimeMeasurement::instance.setname(TwoColorLocator,"TwoColorLocator");
		TimeMeasurement::instance.setname(ApplyOnCC,"TwoColorLocator.ApplyOnCC");
		TimeMeasurement::instance.setname(IntegralImages,"TwoColorLocator.ApplyOnCC.IntegralImages");
		TimeMeasurement::instance.setname(GetCandidateRectangles,"TwoColorLocator.ApplyOnCC.GetCandidateRectangles");
		TimeMeasurement::instance.setname(ProcessIntegralImages,"TwoColorLocator.ApplyOnCC.ProcessIntegralImages");
		TimeMeasurement::instance.setname(ConsolidateRectangles,"TwoColorLocator.ConsolidateRectangles");

		TimeMeasurement::instance.setname(LocateMarkers,"LocateMarkers");
		TimeMeasurement::instance.setname(MarkerScanlines,"LocateMarkers.MarkerScanlines");
		TimeMeasurement::instance.setname(MarkerFitEllipses,"LocateMarkers.MarkerFitEllipses");
		TimeMeasurement::instance.setname(MarkerScanEllipses,"LocateMarkers.MarkerScanEllipses");
		TimeMeasurement::instance.setname(ConsolidateValidate,"LocateMarkers.ConsolidateValidate");
		TimeMeasurement::instance.setname(ShowImages,"ShowImages");
		TimeMeasurement::instance.setname(InterFrameDelay,"InterFrameDelay");
		TimeMeasurement::instance.setname(FullExecution,"FullExecution");
	}
};

#endif