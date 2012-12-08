#ifndef __CONFIGMANAGER_H_
#define __CONFIGMANAGER_H_

#include <iostream>	// only for debug...
#include "../simpleini/SimpleIni.h"

class ConfigManager
{
	static ConfigManager *instance;	// singleton instance (created on stack)

	CSimpleIniA ini;

public:
	static ConfigManager *Current()
	{
		if (instance == NULL)
		{
			instance = new ConfigManager();	// Will be destructed upon program termination
		}
		return instance;
	}

	bool init(char *filename);

	// Config settings
	bool verboseColorCodedFrame;
	bool verboseTwoColorLocator;
	bool verboseLineScanning;
	bool verboseEllipseFitting;
	bool verboseEllipseScanning;
	bool verboseMarkerCodeValidation;
	bool verboseRectConsolidation;
	bool verboseTxt_LineRejectionReason;

	bool showMarkerCodeOnImage;
	bool showInputImage;

	bool resizeImage;
	bool waitFor25Fps;
};

#endif
