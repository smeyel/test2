#ifndef __DETECTIONRESULTEXPORTERBASE_H
#define __DETECTIONRESULTEXPORTERBASE_H

#include "MarkerBase.h"

namespace TwoColorCircleMarker
{
	class DetectionResultExporterBase
	{
	public:
		virtual void writeResult(MarkerBase *marker) = 0;
	};
}

#endif
