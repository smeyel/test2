#ifndef __DETECTIONCOLLECTOR_H
#define __DETECTIONCOLLECTOR_H
#include <iostream>
#include "DetectionResultExporterBase.h"
#include "MarkerBase.h"
#include "Camera.h"

using namespace std;
using namespace TwoColorCircleMarker;

class DetectionCollector : public TwoColorCircleMarker::DetectionResultExporterBase
{
public:
	vector<Ray> rays;
	ofstream stream;

	int currentFrameIdx;
	Camera *cam;

	DetectionCollector()
	{
		cam = NULL;
		rays.clear();
	}

	// Output file operations
	void open(const char *filename)	// Should be called before starting export...
	{
		stream.open(filename);
	}

	void close()
	{
		stream.flush();
		stream.close();
	}

	virtual void writeResult(MarkerBase *marker)
	{
		OPENCV_ASSERT(cam!=NULL,"DetectionCollector.writeResult","Cam pointer is NULL, coordinate system is not known.");

		//cout << "Marker in cam " << cam->cameraID << endl;

		// Create Ray (3D)
		Ray newRay = cam->pointImg2World(marker->center);
		rays.push_back(newRay);

		// Write results to output file (newRay.cameraID is CAMERAID_WORLD)
		stream << newRay.originalCameraID << ";" << currentFrameIdx << ";" <<
			newRay.A.val[0] << ";" << newRay.A.val[1] << ";" << newRay.A.val[2] << ";" << newRay.A.val[3] << ";" <<
			newRay.B.val[0] << ";" << newRay.B.val[1] << ";" << newRay.B.val[2] << ";" << newRay.B.val[3] << std::endl;
	}

	void ShowRaysInFrame(Mat& frame, Camera& cam)
	{
		// Shows all rays in the frame using cam's coordinate system.
		for(int i=0; i<rays.size(); i++)
		{
			Ray& rayWorld = rays[i];
			rayWorld.originalCameraID = CAMID_WORLD;	// Warning, this is cheating!
			Ray rayCam = cam.rayWorld2Cam(rayWorld);
			if(rayCam.A.val[2]<2)
			{
				rayCam.A.val[2] = 2;
			}
			Ray2D ray2D = cam.rayCam2Img(rayCam);
			line(frame,ray2D.A,ray2D.B,Scalar(255,0,0));
		}
	}


};


#endif
