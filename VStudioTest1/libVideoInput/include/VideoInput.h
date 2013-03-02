#ifndef __VIDEOINPUT_H
#define __VIDEOINPUT_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>	// for OPENCV_ASSERT

/** Wrapper for generic video inputs
*/
class VideoInput
{
	public:
		void virtual init(int camID=0) { OPENCV_ASSERT(false,"VideoInput.init(int)","This function should be overridden..."); }
		void virtual init(const char *filename) { OPENCV_ASSERT(false,"VideoInput.init(char*)","This function should be overridden..."); }
		bool virtual captureFrame(cv::Mat &frame)  { return false; }
		void virtual release() { }

		/** Increments given camera parameter by 10
			@param parameter	The parameter to increment.
								Typical parameters: CLEYE_GAIN CLEYE_EXPOSURE CLEYE_ZOOM
			@returns			The new value of the parameter
		*/
		int IncrementCameraParameter(int param) { return -1; }

		/** Decrements given camera parameter by 10
			@param parameter	The parameter to increment.
								Typical parameters: VIDEOINPUTPS3EYEPARAMETERS_GAIN VIDEOINPUTPS3EYEPARAMETERS_EXPOSURE
								(Found in VideoInputPs3EyeParameters.h)
			@returns			The new value of the parameter
		*/
		int DecrementCameraParameter(int param) { return -1; }

		~VideoInput()
		{
			// TODO: this does not seem to invoke the correct release() function, only the one of this base class...
			this->release();
		}
};
		
#endif
