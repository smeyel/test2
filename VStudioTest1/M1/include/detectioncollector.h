#ifndef __DETECTIONCOLLECTOR_H
#define __DETECTIONCOLLECTOR_H
#include "DetectionResultExporterBase.h"
#include "MarkerBase.h"

using namespace TwoColorCircleMarker;

class DetectionCollector : public TwoColorCircleMarker::DetectionResultExporterBase
{
public:
	int currentFrameIdx;
	int currentCamID;

	virtual void writeResult(MarkerBase *marker)
	{
/*		stream << "FID:" << currentFrameIdx << ",CID:" << currentCamID << " ";
		marker->exportToTextStream(&stream);
		stream << endl; */
	}
};


#endif
