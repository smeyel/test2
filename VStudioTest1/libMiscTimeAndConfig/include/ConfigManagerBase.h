#ifndef __CONFIGMANAGERBASE_H_
#define __CONFIGMANAGERBASE_H_

#include <iostream>	// only for debug...
#include "SimpleIni.h"

namespace MiscTimeAndConfig
{
	/** Used to store config data read from a configuration (ini) file.
		To use it, override readConfiguration to read the config data from the ini file.
	*/
	class ConfigManagerBase
	{
		CSimpleIniA ini;

		/** This method is called by init of the base class to read the configuration values.
			Override it to read the value of the additional config properfies from the ini file.
		*/
		virtual bool readConfiguration(CSimpleIniA *ini);

	public:
		/** Do not override, just call it. It initializes SimpleIni and calles
			your readConfiguration so it can get the config data and store them in properties.
		*/
		bool init(char *filename);
	};
}

#endif
