#ifndef __CONFIGMANAGER_H_
#define __CONFIGMANAGER_H_

#include "ConfigManagerBase.h"


namespace TwoColorCircleMarker
{
	class ConfigManager : public MiscTimeAndConfig::ConfigManagerBase
	{
		// This method is called by init of the base class to read the configuration values.
		virtual bool readConfiguration(CSimpleIniA *ini);

	public:
		static ConfigManager *Current()
		{
			if (instance == NULL)
			{
				instance = new ConfigManager();	// Will be destructed upon program termination
			}
			return (ConfigManager*)instance;
		}

		// Config settings
		bool verboseColorCodedFrame;
		bool verboseTwoColorLocator;
		bool verboseLineScanning;
		bool verboseEllipseFitting;
		bool verboseEllipseScanning;
		bool verboseMarkerCodeValidation;
		bool verboseRectConsolidationResults;
		bool verboseTxt_LineRejectionReason;
		bool verboseTxt_RectConsolidation;
		bool verboseTxt_RectConsolidationSummary;
		bool verboseOverlapMask;
		bool verboseTwoColorLocatorIntegralReject;

		bool showMarkerCodeOnImageDec;
		bool showMarkerCodeOnImageHex;
		bool showInputImage;

		bool resizeImage;
		bool waitFor25Fps;
		bool pauseIfNoValidMarkers;
	};
}

#endif