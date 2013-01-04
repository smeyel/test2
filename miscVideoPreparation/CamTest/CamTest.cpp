#include <iostream>	// for standard I/O
#include <string>   // for strings

#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write

using namespace std;
using namespace cv;

int main()
{
	VideoCapture inputVideo(0);        // Open input
	if ( !inputVideo.isOpened())
	{
		cout  << "Could not open the default camera (ID 0)..." << endl;
		return -1;
	}

	namedWindow("Video", CV_WINDOW_AUTOSIZE);

	Mat src;
	int framecounter = 0;	// 0-based frame counter
	int jumpFrameNum = 0;	// Jump this number of frames without user interaction
	bool finished = false;
	while(!finished)
	{
		inputVideo >> src;              // read
		if( src.empty()) 
		{
			break;         // check if at end
		}
		//cout << "Frame " << framecounter << "\r";

		// Draw frame number on frame
		char txt[10];
		sprintf(txt,"%d",framecounter);
		putText(src,string(txt),Point(20,20),FONT_HERSHEY_PLAIN,1,Scalar(0,255,0),1);

		// Increase frame counter
		framecounter++;

		// Show frame (if not in fast-forward mode)
		imshow("Video",src);

		// Wait for user command
		char c = cvWaitKey(25);
		switch (c)
		{
		case -1:	// Nothing pressed...
			break;
		case 'x':
			finished=true;
			break;
		default:
			cout
				<< "Control keys:"  << endl
				<< "SPACE   jump to next frame"  << endl
				<< "    1   jump 1 second (25 frames)"  << endl
				<< "    5   jump 5 seconds (5*25=125 frames)"  << endl
				<< "    x   exit program"  << endl;
			break;
		}
	}
	return 0;
}
