#ifndef __DETECTIONRESULTEXPORTERBASE_H
#define __DETECTIONRESULTEXPORTERBASE_H

#include "MarkerBase.h"

namespace TwoColorCircleMarker
{
	/** Base class which is used by MarkerCC2Locator to export the detection results to.
	*/
	class DetectionResultExporterBase
	{
	public:
		/** Called for every detection, so it can be stored.
			Warning: the MarkerBase instance *marker may be overwritten
				after the call. Copy it before return.
		*/
		virtual void writeResult(MarkerBase *marker) = 0;
	};
}

#endif
