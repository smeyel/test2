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

	int currentFrameIdx;
	Camera *cam;

	DetectionCollector()
	{
		cam = NULL;
		rays.clear();
	}

	virtual void writeResult(MarkerBase *marker)
	{
		OPENCV_ASSERT(cam!=NULL,"DetectionCollector.writeResult","Cam pointer is NULL, coordinate system is not known.");

		cout << "Marker in cam " << cam->cameraID << endl;

		// Create Ray (3D)
		Ray newRay = cam->pointImg2World(marker->center);
		rays.push_back(newRay);
	}

	void startNewFrame()
	{
		rays.clear();
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
