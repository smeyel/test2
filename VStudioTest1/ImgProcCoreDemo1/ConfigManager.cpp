#include <iostream>
#include <assert.h>

#include "ConfigManager.h"

ConfigManager *ConfigManager::instance;

bool ConfigManager::init(char *filename)
{
    // load from a data file
    SI_Error rc = ini.LoadFile(filename);
	assert(rc >= 0);

	verboseColorCodedFrame = ini.GetBoolValue("verbose","verboseColorCodedFrame",false,NULL);
	verboseTwoColorLocator = ini.GetBoolValue("verbose","verboseTwoColorLocator",false,NULL);
	verboseLineScanning = ini.GetBoolValue("verbose","verboseLineScanning",false,NULL);
	verboseEllipseFitting = ini.GetBoolValue("verbose","verboseEllipseFitting",false,NULL);
	verboseEllipseScanning = ini.GetBoolValue("verbose","verboseEllipseScanning",false,NULL);
	verboseMarkerCodeValidation = ini.GetBoolValue("verbose","verboseMarkerCodeValidation",false,NULL);
	verboseRectConsolidation = ini.GetBoolValue("verbose","verboseRectConsolidation",false,NULL);

	showMarkerCodeOnImage = ini.GetBoolValue("show","showMarkerCodeOnImage",false,NULL);
	showInputImage = ini.GetBoolValue("show","showInputImage",false,NULL);

	resizeImage = ini.GetBoolValue("settings","resizeImage",false,NULL);
	waitFor25Fps = ini.GetBoolValue("settings","waitFor25Fps",false,NULL);

	return true;
}
