#ifndef __MARKERCC2_H_
#define __MARKERCC2_H_
#include <ostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include "MarkerBase.h"
#include "MarkerCC2Locator.h"

#include "TimeMeasurement.h"
#include "ConfigManagerBase.h"

using namespace cv;

namespace TwoColorCircleMarker
{
	/** THis class represents a marker of type CC2 (Color Circle 2)
		To use it,
		- call static init() once
		- set markerLocator
		- optionally set verboseImage
		- call readCode

	*/
	class MarkerCC2 : public MarkerBase
	{
		// Internal configuration class
		class ConfigManager : public MiscTimeAndConfig::ConfigManagerBase
		{
			// This method is called by init of the base class to read the configuration values.
			virtual bool readConfiguration(CSimpleIniA *ini);

		public:
			bool showMarkerCodeOnImageDec;
			bool showMarkerCodeOnImageHex;

			bool verboseEllipseFitting;
			bool verboseEllipseScanning;

			bool verboseLineScanning;
			bool verboseTxt_LineRejectionReason;

			bool verboseMarkerCodeValidation;
			bool verboseTxt_MarkerCodeValidation;
		};

		static ConfigManager configManager;

	public:
		/** static init
			@param configFileName	name of the configuration file.
		*/
		static void init(char *configFileName);

		/** ID of the marker. Set by readCode(). Only valid if isValid is true.
		*/
		int MarkerID;

		/** True if the MarkerID could be read and is a valid ID.
		*/
		bool isValid;

		/** Direction (in degrees) of the center of the green area.
			Warning: this value has low resolution!
		*/
		float orientationReferenceAngle;

		/** True if orientationReferenceAngle is valid.
			Invalid means the direction could not be found.
		*/
		bool isOrientationReferenceAngleValid;

		/** Set to a BGR image to visualize verbose information.
			Verbose functions can be enabled in the configuration file.
		*/
		Mat *verboseImage;

		/** Marker locator used to validate the read MarkerID.
		*/
		MarkerCC2Locator *markerLocator;

		/** Constructor
		*/
		MarkerCC2()
		{
			MarkerID=0;
			isValid=false;
			isCenterValid=false;
			verboseImage = NULL;
			markerLocator = NULL;
			orientationReferenceAngle = 0.0;
			isOrientationReferenceAngleValid = false;
		}

		/** Read the marker code for a given candidate rectangle.
			If the read is successful, it is really a valid marker.
			Called by marker locator to read the code of the marker.

			@param srcCC	Color code image
			@param rect		Rectangle of the marker location (inner blue area)
		*/
		void readCode(Mat &srcCC, Rect &rect);

		/** Export marker information in human readable format to a stream
			Used by detection exporter classes like DetectionResultExporter.
		*/
		virtual void exportToTextStream(std::ostream *stream);

	private:
		// --- Used by ellpise fitting
		int scanDistance;	// length of scan starting from center (manhattan distance)
		// Scans along a line for the borders of the red circle
		bool findBordersAlongLine(Mat &srcCC, int dir);
		// Returns direction in degrees for given bit of marker ID
		float bitIdx2Angle(int bitIdx);
		// Returns the endpoint of a line in a given direction
		CvPoint getEndPoint(int x, int y, int distance, int dir);
		// Fits an ellipse on the inner and outer borders of the red circle
		//	based on the results of findBordersAlongLine calls.
		void fitBorderEllipses();
		// Variables used during markercode reading.
		// May be used for color recognition adjustments.
		Point RedInnerBorders[8];
		Point RedOuterBorders[8];
		RotatedRect innerEllipse;
		RotatedRect outerEllipse;

		// --- Reading marker code areas
		void scanEllipses(Mat &srcCC);
		Point getEllipsePointInDirection(RotatedRect baseEllipse,float directionAngle,float distanceMultiplier, Mat &srcCC);
		// For verbose purposes, stores location of the ellipse scan points for every bit
		Point bitLocations[32];	// 4x8 ellipse points
		uchar rawMarkerIDBitCC[32];	// Color code values of the scanned points

		// --- Marker code processing
		void validateAndConsolidateMarkerCode();
	};
}

#endif
