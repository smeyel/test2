#ifndef __MYCONFIGMANAGER_H
#define __MYCONFIGMANAGER_H
#include "ConfigManagerBase.h"

class MyConfigManager : public MiscTimeAndConfig::ConfigManagerBase
{
	// This method is called by init of the base class to read the configuration values.
	virtual bool readConfiguration(CSimpleIniA *ini)
	{
		showInputImage = ini->GetBoolValue("Main","showInputImage",false,NULL);
		return true;
	}

public:
	// --- Settings
	bool showInputImage;
};

#endif

