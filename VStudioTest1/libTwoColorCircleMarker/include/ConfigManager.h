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

		// --- Settings
		bool resizeImage;

		// --- Visualization settings
		// Result visualization
		bool showInputImage;
		bool showMarkerCodeOnImageDec;
		bool showMarkerCodeOnImageHex;
		bool waitFor25Fps;
		bool pauseIfNoValidMarkers;

		// --- Verbose functions
		// Show verbose frames
		bool verboseColorCodedFrame;
		bool verboseOverlapMask;
		// Verbose marker processing (orientation detection, code reading etc.)
		bool verboseLineScanning;
		bool verboseTxt_LineRejectionReason;
		bool verboseEllipseFitting;
		bool verboseEllipseScanning;
		bool verboseMarkerCodeValidation;
		bool verboseTxt_MarkerCodeValidation;
		// Verbose marker localization
		bool verboseRectConsolidationCandidates;
		bool verboseRectConsolidationResults;
		bool verboseTxt_RectConsolidation;
		bool verboseTxt_RectConsolidationSummary;
	};
}

#endif