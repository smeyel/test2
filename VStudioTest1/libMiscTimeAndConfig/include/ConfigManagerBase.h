#ifndef __CONFIGMANAGERBASE_H_
#define __CONFIGMANAGERBASE_H_

#include <iostream>	// only for debug...
#include "SimpleIni.h"

namespace MiscTimeAndConfig
{

	class ConfigManagerBase
	{
		CSimpleIniA ini;

		// This method is called by init of the base class to read the configuration values.
		virtual bool readConfiguration(CSimpleIniA *ini);

	protected:
		static ConfigManagerBase *instance;	// singleton instance (created on stack)

		ConfigManagerBase()	// Private constructor
		{
		}

	public:
		static ConfigManagerBase *Current()
		{
			if (instance == NULL)
			{
				instance = new ConfigManagerBase();	// Will be destructed upon program termination
			}
			return instance;
		}

		bool init(char *filename);
	};
}

#endif
