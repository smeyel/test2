#include <iostream>
#include <sstream>
#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

void main()
{
	// --------------- Load calibration data
    time_t t;
    time( &t );
    struct tm *t2 = localtime( &t );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

	FileStorage fs("test1.xml", FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cout << "Could not open the configuration file..." << endl;
        return;
    }

    Mat cameraMatrix, distCoeffs;
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    distCoeffs = Mat::zeros(8, 1, CV_64F);

    fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distCoeffs;

	for (int r=0; r<distCoeffs.rows; r++)
	{
		for (int c=0; c<distCoeffs.cols; c++)
		{
			cout << "r"<<r<<"c"<<c<<":"<<distCoeffs.at<double>(r,c)<<endl;
		}
	}

    fs.release(); 


}
