#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\core\mat.hpp>
#include "TwoColorLocator.h"
#include "MarkerCC1.h"

using namespace cv;

// Meg kell hivni, mielott az alabbi fuggvenyeket hasznaljuk... (2 bites kodok Hue ertekhez rendelese)
void MarkerCC1::initHue2CodeLut()
{
	for(int i=0; i<256; i++)
	{
		if (123 <= i && i <= 150)	// 125-148	(BLU)
		{
			hue2codeLUT[i] = 0;
		}
		else if (170 <= i && i <= 177)	// 172-175 (RED)
		{
			hue2codeLUT[i] = 1;
		}
		else
		{
			hue2codeLUT[i] = -1;
		}
	}
}

// Entry of marker identification
// It needs a hue image
// Egy potencialis marker helyet dolgoz fel.
// calls: readCodeAlongLine, validateAndConsolidateMarkerCode
void MarkerCC1::readCode(Mat &hueImg, CvRect &rect, Mat *verboseImage)
{
	CV_Assert(hueImg.depth() == CV_8U);	// Assuming every pixel to have 8 bits...
	// TODO: draw on debugImage if not NULL...

	// Init marker
	center.x = rect.x + rect.width/2;
	center.y = rect.y + rect.height/2;
	for(int i=0; i<8; i++) codeArray[i]=0;
	isValid = false;	// Not valid, as not set...

	scanDistance = rect.width>rect.height ? 2*rect.width : 2*rect.height;

	// SCAN
	for (int dir=0; dir<8; dir++)
	{
		if (!readCodeAlongLine(hueImg,dir))
		{
			return;	// Center color reject...
		}
	}

	// TODO: Validate marker code
	validateAndConsolidateMarkerCode();

	char tmpCodeString[255];
	sprintf(tmpCodeString,"%d-%d-%d-%d-%d",codeArray[0],codeArray[1],
		codeArray[2],codeArray[3],codeArray[4]);
	string debugtxt(tmpCodeString);

	if (verboseImage != NULL)
	{
		if (isValid)
		{
//			line(*debugImage,marker.center,endpoint,Scalar(0,255,0));
			putText(*verboseImage,debugtxt,center,FONT_HERSHEY_PLAIN,1,Scalar(0,255,0),2);
		}
		else
		{
//			line(*debugImage,marker.center,endpoint,Scalar(0,0,255));
			putText(*verboseImage,debugtxt,center,FONT_HERSHEY_PLAIN,1,Scalar(0,0,255),2);
		}
	}
}


// Egy iranybe vegigolvassa a szineket (meg jo esellyel kicsit tovabb is.)
// Kitolti a marker.codeArray -t.
// Performs center color validation!
// Return: false for immediate reject, True: may be valid marker...
bool MarkerCC1::readCodeAlongLine(Mat &img, int dir)
{
	// Szinenkent 2 bitet kodolunk
	// Max. MaxValuePerLine db eltero es felismert szint olvasunk be egymas utan a vonal menten.
	const int MaxValuePerLine = 4;	// max. 2x4=8 bitet hasznalunk el minden iranyban (meg nem nezzuk, meddig hasznos!)

	// TODO: vajon van annyi esze, hogy nem megy ki a kepbol?!
	CvPoint endpoint = getEndPoint(center.x, center.y, scanDistance, dir);

	// Scan along the given line and store the color codes (using hue2codeLUT)
	LineIterator lineIt(img,center,endpoint);
	int length = lineIt.count;
	
	uchar values[200];	// No dynamic memory allocation...
	if (length>199) length=199;

	for(int i=0; i<length; i++,lineIt++)
	{
		uchar value = *lineIt.ptr;
		values[i] = hue2codeLUT[value];	// Itt -1 ervenytelen, ervenyes: 0-3 (2 bit/szin)

		// Center must be blue, otherwise marker is definitely invalid
		if (i==0 && values[i] != 0)	// 0 érték: BLUE (marker center color)
		{
			return false;	// Center color reject
		}
	}

	// zero out sequences shorter than 3 pixels
	// preserve only one value per sequence
	uchar prev = values[0];
	int seqLength = 0;
	int nextValueIdx = 0;
	unsigned int resultCode = 0;
	for(int i=0; i<length; i++)
	{
		if (values[i] == prev)
		{
			// Sequence is continuing...
			seqLength++;
		}
		else
		{
			// End of sequence, was it long enough to store?
			// -1 is invalid color...
			if (seqLength > 3 && prev != -1)
			{
				if ( nextValueIdx < MaxValuePerLine)
				{
					// prev ertekenek mentese a kodba, megfeleloen beshift-elve a helyere azt a 2 bitet...
					// Starting with LSB at center.
					resultCode |= prev << (2*nextValueIdx);
					nextValueIdx++;
				}
			}

			// Need to update prev...
			prev = values[i];
		}
	}
	// Eredmény mentése a marker-be... (a végén még lehetnek hibás bitek!)
	// Meg igazabol a kozeppont szinet is ki lehet dobni az LSB-nel levo 2 bitrol...
	codeArray[dir]=resultCode;
	return true;
}

CvPoint MarkerCC1::getEndPoint(int x, int y, int distance, int dir)
{
	CV_Assert(dir>=0 && dir<8);

	const int dirXMul[] = {0,1,1,1,0,-1,-1,-1};
	const int dirYMul[] = {-1,-1,0,1,1,1,0,-1};

	// TODO: vajon van annyi esze, hogy nem megy ki a kepbol?!
	CvPoint endpoint = cvPoint(
		x + dirXMul[dir]*distance,
		y + dirYMul[dir]*distance);

	return endpoint;
}

// Ha mar beolvastuk a szineket a 8 iranyba, akkor ez validalja es szamitja ki a martkerID-t.
void MarkerCC1::validateAndConsolidateMarkerCode()
{
	isValid = true;

	// TODO: pl. melyik iranyok voltak ervenyesek? Valasszunk ki 4-et a 8-bol!!!
	// Ezt csak 2 fele keppen lehet. Az egyik lehet, hogy invalid, akkor jon a masik...
	// Belso kek 2 bitjet ki lehet shift-elni, az nem kell.
	// Max. hany gyuru van? Annyiszor 2 bit kellhet max...

	// Ha 2 gyuru van, iranyonkent 4 bit adat lesz. Vagyis a teljes kod 4x4=16 bit,
	// de: az iranyok sorrendje ismert, a kezdoirany viszont nem!

	uchar resultCodes[8];

	for(int dir=0; dir<8; dir++)
	{
		// 2 szingyuru marad, kozepso kek kiesik
		resultCodes[dir] = (codeArray[dir] & 0x3C) >> 2;
	}

	// TODO: vegso soron a marker.markerID -t kell beallitani.
}

