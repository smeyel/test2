#ifndef __DETECTIONRESULTEXPORTERBASE_H
#define __DETECTIONRESULTEXPORTERBASE_H

namespace MiscTimeAndConfig
{
	class DetectionResultExporterBase
	{
	public:
		virtual void writeResult(int markerID, int frameX, int frameY, bool isCenterValid, bool isMarkerCodeValid) = 0;
	};
}

#endif
