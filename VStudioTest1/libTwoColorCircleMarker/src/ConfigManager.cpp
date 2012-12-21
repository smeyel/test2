#include <iostream>
#include <assert.h>

#include "ConfigManager.h"
using namespace TwoColorCircleMarker;

bool ConfigManager::readConfiguration(CSimpleIniA *ini)
{
	verboseTwoColorLocator = ini->GetBoolValue("verbose","verboseTwoColorLocator",false,NULL);
	verboseLineScanning = ini->GetBoolValue("verbose","verboseLineScanning",false,NULL);
	verboseEllipseFitting = ini->GetBoolValue("verbose","verboseEllipseFitting",false,NULL);
	verboseEllipseScanning = ini->GetBoolValue("verbose","verboseEllipseScanning",false,NULL);
	verboseMarkerCodeValidation = ini->GetBoolValue("verbose","verboseMarkerCodeValidation",false,NULL);
	verboseRectConsolidationResults = ini->GetBoolValue("verbose","verboseRectConsolidationResults",false,NULL);
	verboseRectConsolidationCandidates = ini->GetBoolValue("verbose","verboseRectConsolidationCandidates",false,NULL);
	verboseOverlapMask = ini->GetBoolValue("verbose","verboseOverlapMask",false,NULL);

	verboseTxt_LineRejectionReason = ini->GetBoolValue("verbosetxt","LineRejectionReason",false,NULL);
	verboseTxt_RectConsolidation = ini->GetBoolValue("verbosetxt","RectConsolidation",false,NULL);
	verboseTxt_RectConsolidationSummary = ini->GetBoolValue("verbosetxt","RectConsolidationSummary",false,NULL);
	verboseTwoColorLocatorIntegralReject = ini->GetBoolValue("verbosetxt","TwoColorLocatorIntegralReject",false,NULL);

	showMarkerCodeOnImageDec = ini->GetBoolValue("show","showMarkerCodeOnImageDec",false,NULL);
	showMarkerCodeOnImageHex = ini->GetBoolValue("show","showMarkerCodeOnImageHex",false,NULL);
	showInputImage = ini->GetBoolValue("show","showInputImage",false,NULL);
	verboseColorCodedFrame = ini->GetBoolValue("show","ColorCodedFrame",false,NULL);

	resizeImage = ini->GetBoolValue("settings","resizeImage",false,NULL);
	waitFor25Fps = ini->GetBoolValue("settings","waitFor25Fps",false,NULL);
	pauseIfNoValidMarkers = ini->GetBoolValue("settings","pauseIfNoValidMarkers",false,NULL);

	return true;	// Successful
}
