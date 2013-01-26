#ifndef __DETECTIONCOLLECTOR_H
#define __DETECTIONCOLLECTOR_H
#include <iostream>
#include "DetectionResultExporterBase.h"
#include "MarkerBase.h"

using namespace std;
using namespace TwoColorCircleMarker;

class DetectionCollector : public TwoColorCircleMarker::DetectionResultExporterBase
{
public:
	int currentFrameIdx;
	int currentCamID;

	virtual void writeResult(MarkerBase *marker)
	{
		cout << "FID:" << currentFrameIdx << ",CID:" << currentCamID << " ";
		marker->exportToTextStream(&cout);
		cout << endl;
	}
};


#endif
