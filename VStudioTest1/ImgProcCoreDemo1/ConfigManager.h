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
	bool twocolorlocator_verbose;
	bool marker_show;
	bool marker_verbose;

	bool marker_verboseScanlines;
	bool marker_verboseValidation;
};

#endif
