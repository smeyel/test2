#ifndef __MARKERBASE_H_
#define __MARKERBASE_H_
#include <ostream>

namespace TwoColorCircleMarker
{
	/** Base class for a marker. DetectionResultExporterBase can receive this kind of markers.
	*/
	class MarkerBase
	{
	public:
		/** Location of the marker center.
		*/
		Point2d center;

		/** Is the center location valid?
		*/
		bool isCenterValid;	// Center is located. May be true if MarkerID is not valid.

		/** Export marker information in human readable format to a stream
		*/
		virtual void exportToTextStream(std::ostream *stream) = 0;
	};
}

#endif

