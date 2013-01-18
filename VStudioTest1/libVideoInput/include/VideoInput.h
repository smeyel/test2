#ifndef __VIDEOINPUT_H
#define __VIDEOINPUT_H

#include <opencv2/core/core.hpp>

/** Wrapper for generic video inputs
*/
class VideoInput
{
	public:
		void virtual init() { }
		bool virtual captureFrame(cv::Mat &frame)  { return false; }
		void virtual release() { }

		~VideoInput()
		{
			release();
		}
};
		
#endif
