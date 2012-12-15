#ifndef __MARKERBASE_H_
#define __MARKERBASE_H_
#include <ostream>

namespace TwoColorCircleMarker
{
	class MarkerBase
	{
	public:
		Point2d center;
		bool isCenterValid;	// Center is located. May be true if MarkerID is not valid.

		// Export marker information in human readable format to a stream
		virtual void exportToTextStream(std::ostream *stream) = 0;
	};
}

#endif

