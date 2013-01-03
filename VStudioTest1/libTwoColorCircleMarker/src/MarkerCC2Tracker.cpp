#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>

using namespace cv;

namespace TwoColorCircleMarker
{

	// THis class wraps all functions related to MarkerCC2 markers:
	// - localization on a frame
	// - estimation of position
	// - handling validation of marker codes
	// - handling multiple video sources
	class MarkerCC2Tracker
	{
	private:
		// Functions called by processFrame()



	public:
		// Constructor
		MarkerCC2Tracker();

		// Interface for processing a new frame
		// Contains: color filtering, marker localization
		//	(accelerated by location predicion if available)
		// Automatically adds tracking data to results from previous frames
		void processFrame(Mat &src, int cameraID, float timestamp);
	}




}