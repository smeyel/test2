#ifndef __VIDEOINPUTFACTORY_H
#define __VIDEOINPUTFACTORY_H
#include "VideoInput.h"

#define VIDEOINPUTTYPE_GENERIC	0
#define VIDEOINPUTTYPE_PS3EYE	1

class VideoInputFactory
{
public:
	static VideoInput *CreateVideoInput(int VideoInputType);
};

#endif
