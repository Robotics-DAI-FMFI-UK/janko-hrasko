#ifndef _PP_CAMERA_H
#define _PP_CAMERA_H

#ifndef _PP_CAMERA_CALIBRATION_H
#include "cameraCalibration.h"
#endif

using namespace std;
using namespace cv;

//--------------------------------------------------------------------------------------------------------------

class Camera{
private:
	int idCameraChannel;					// -1 is any camera, 0 ... n are particular cameras
	VideoCapture capture;					// cvClass for handling image or video capturing

	Mat map1, map2;							// values for image calibration, input to cvFunction remap(...)
	Size imageSize, calibratedImageSize;	// cvSize is record - size.width and size.height

	bool initialized;						// bool if camera is properly calibrated

public:
	Camera() : idCameraChannel(-1), initialized(false) {};
	Camera(int channel) : idCameraChannel(channel), initialized(false){};

	bool init(int channel, int width, int height, string& statusMsg);		// initialization of camera

	bool readCalibratedMatrices(string& statusMsg);							// read already calibrated matrices from input file, set map1, map2 and calibratedSize
	bool calibrateCamera(string& statusMsg);								// calibrate camera, write properties to output file, and set map1, map2 and calibratedSize


	bool grabCaptureImage(Mat& view, bool calibrateCapturedImage);			// 
	
	void calibrateImage(Mat& view, Mat* rview);								// rview is NULL by default, if not provided, original Mat view will be calibrated, if provided Mat view will be left unchanged and MAt rview will be calibrated


};

#endif