#include "VideoInputFactory.h"

#include "VideoInputGeneric.h"
#include "VideoInputPs3Eye.h"

#define VIDEOINPUTTYPE_GENERIC	0
#define VIDEOINPUTTYPE_PS3EYE	1

VideoInput *VideoInputFactory::CreateVideoInput(int videoInputType)
{
	switch (videoInputType)
	{
	case VIDEOINPUTTYPE_GENERIC:
		return new VideoInputGeneric();
	case VIDEOINPUTTYPE_PS3EYE:
		return new VideoInputPs3Eye();
	}
	return NULL;
}
