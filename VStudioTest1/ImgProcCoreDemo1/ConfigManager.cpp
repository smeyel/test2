#include <iostream>	// only for debug...

#include "ConfigManager.h"

ConfigManager *ConfigManager::instance;

bool ConfigManager::init(char *filename)
{
    // load from a data file
    SI_Error rc = ini.LoadFile(filename);
    if (rc < 0) return false;

/*    // get all sections
    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections); */

/*    // get all keys in a section
    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys("section-name", keys); */

    // GETTING VALUES

	twocolorlocator_verbose = ini.GetBoolValue("verbose","twocolorlocator",false,NULL);
	marker_verbose = ini.GetBoolValue("verbose","marker",false,NULL);
	marker_show = ini.GetBoolValue("show","marker",false,NULL);
//	marker_verboseEllipses = ini.GetBoolValue("verbose","marker_verboseEllipses",false,NULL);
	marker_verboseScanlines = ini.GetBoolValue("verbose","marker_verboseScanlines",false,NULL);
//	marker_verboseRect = ini.GetBoolValue("verbose","marker_verboseRect",false,NULL);
	marker_verboseValidation = ini.GetBoolValue("verbose","marker_verboseValidation",false,NULL);

    // get the value of a key
    const char * pszValue = ini.GetValue("testsection",
        "testkeyname", NULL);

	std::cout << "ConfigManager.init: read value " << pszValue << std::endl;

	return true;
}
