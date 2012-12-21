#include <iostream>
#include <assert.h>

#include "ConfigManager.h"
using namespace TwoColorCircleMarker;

bool ConfigManager::readConfiguration(CSimpleIniA *ini)
{
	resizeImage = ini->GetBoolValue("Settings","ResizeImage",false,NULL);

	showInputImage = ini->GetBoolValue("Show","InputImage",false,NULL);
	showMarkerCodeOnImageDec = ini->GetBoolValue("Show","MarkerCodeOnImageDec",false,NULL);
	showMarkerCodeOnImageHex = ini->GetBoolValue("Show","MarkerCodeOnImageHex",false,NULL);
	verboseColorCodedFrame = ini->GetBoolValue("Show","ColorCodedFrame",false,NULL);
	verboseOverlapMask = ini->GetBoolValue("Show","OverlapMask",false,NULL);
	waitFor25Fps = ini->GetBoolValue("Show","WaitFor25Fps",false,NULL);
	pauseIfNoValidMarkers = ini->GetBoolValue("Show","PauseIfNoValidMarkers",false,NULL);

	verboseLineScanning = ini->GetBoolValue("VerboseMarker","LineScanning",false,NULL);
	verboseEllipseFitting = ini->GetBoolValue("VerboseMarker","EllipseFitting",false,NULL);
	verboseEllipseScanning = ini->GetBoolValue("VerboseMarker","EllipseScanning",false,NULL);
	verboseMarkerCodeValidation = ini->GetBoolValue("VerboseMarker","MarkerCodeValidation",false,NULL);
	verboseTxt_LineRejectionReason = ini->GetBoolValue("VerboseMarkerTxt","LineRejectionReason",false,NULL);
	verboseTxt_MarkerCodeValidation = ini->GetBoolValue("VerboseMarkerTxt","MarkerCodeValidation",false,NULL);

	verboseRectConsolidationCandidates = ini->GetBoolValue("VerboseMarkerLocalization","MarkerRectCandidates",false,NULL);
	verboseRectConsolidationResults = ini->GetBoolValue("VerboseMarkerLocalization","RectConsolidationResults",false,NULL);
	verboseTxt_RectConsolidation = ini->GetBoolValue("VerboseMarkerLocalizationTxt","RectConsolidation",false,NULL);
	verboseTxt_RectConsolidationSummary = ini->GetBoolValue("VerboseMarkerLocalizationTxt","RectConsolidationSummary",false,NULL);

	return true;	// Successful
}
