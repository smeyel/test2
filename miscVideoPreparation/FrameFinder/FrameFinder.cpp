#include <iostream>	// for standard I/O
#include <string>   // for strings

#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write

using namespace std;
using namespace cv;

void help()
{
	cout
		<< "\n--------------------------------------------------------------------------" << endl
		<< "This program shows a video frame-by-frame to help you locate important frames."
		<< "Control keys:"  << endl
		<< "SPACE   jump to next frame"  << endl
		<< "    1   jump 1 second (25 frames)"  << endl
		<< "    5   jump 5 seconds (5*25=125 frames)"  << endl
		<< "    x   exit program"  << endl
		<< "Usage:"                                                               << endl
		<< "./FrameFinder inputvideoname " << endl
		<< "--------------------------------------------------------------------------"   << endl
		<< endl;
}

int main(int argc, char *argv[], char *window_name)
{
	help();
	if (argc != 2)
	{
		return -1;
	}
	const string source = argv[1];            // the source file name

//	const string source = "d:\\SMEyeL\\inputmedia\\MultiViewVideo1\\2011-10-01-017.mp4";

	VideoCapture inputVideo(source);        // Open input
	if ( !inputVideo.isOpened())
	{
		cout  << "Could not open the input video: " << source << endl;
		return -1;
	}

	int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

	cout << "Input frame number " << inputVideo.get(CV_CAP_PROP_FRAME_COUNT) << endl;

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
		cout << "Frame " << framecounter << "\r";

		// Draw frame number on frame
		char txt[10];
		sprintf(txt,"%d",framecounter);
		putText(src,string(txt),Point(20,20),FONT_HERSHEY_PLAIN,1,Scalar(0,255,0),1);

		// Increase frame counter
		framecounter++;

		// If running forward, start next iteration
		jumpFrameNum--;
		if (jumpFrameNum > 0)
		{
			continue;
		}

		// Show frame (if not in fast-forward mode)
		imshow("Video",src);

		// Wait for user command
		char c = cvWaitKey(0);
		switch (c)
		{
		case ' ':
			jumpFrameNum = 1;	// Jump 1 frame
			break;
		case '1':
			jumpFrameNum = 25;	// Jump 25 frames
			break;
		case '5':
			jumpFrameNum = 125;	// Jump 125 frames
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
