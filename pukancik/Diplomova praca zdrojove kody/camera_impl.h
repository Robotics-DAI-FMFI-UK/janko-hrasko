#ifndef _PP_CAMERA_IMPl_H
#define _PP_CAMERA_IMPl_H

#ifndef _PP_CAMERA_H
#include "camera.h"
#endif

using namespace std;
using namespace cv;

// ----------------------------------------------------------------------------------------------------------------------

// msg je parameter, ktory v pripade false nepodarku vrati popis dovodu falsu
bool Camera::init(int channel, int width, int height, string& statusMsg){
	// default settings
	calibratedImageSize = Size(0, 0);
	
	// init VideoCapture from input
	idCameraChannel = channel;
	capture = VideoCapture(idCameraChannel);

	// test
	if(!capture.isOpened()){
		statusMsg = "Camera init failed: Reason:\n  Camera not found...";
        return false;
	}

	// set VideoCapture properties - width and height
	imageSize.width = width;
	imageSize.height = height;

	capture.set(CV_CAP_PROP_FRAME_WIDTH, imageSize.width);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, imageSize.height);

	// bud len init camera map1 a map2 a calibratedImageSize, alebo najskor predtym aj kalibruj 
	initialized = readCalibratedMatrices(statusMsg);

	// if precalibrated values haven't read and set for some reason, try to calibrate camera 
	if(!initialized){
		initialized = calibrateCamera(statusMsg);
	}
	
	// test - if calibrated values are the same properties (width, height) as actual camera properties (width, height)
	if((calibratedImageSize.width != imageSize.width)||(calibratedImageSize.height != imageSize.height)){
		ostringstream msg_stream;
		msg_stream << "Camera init failed: Reason:" << endl;
		msg_stream << "Camera is not calibrated for this imageSize. \nCurrently it's calibrated to width = ";
		msg_stream << calibratedImageSize.width; msg_stream << " and height = "; msg_stream << calibratedImageSize.height;
		msg_stream << endl << "Your input was set with these parameters width = "; msg_stream << imageSize.width;
		msg_stream << " and height = "; msg_stream << imageSize.height << endl;

		statusMsg = msg_stream.str();
		return false;
	}

	return initialized;
}

bool Camera::readCalibratedMatrices(string& statusMsg){
	// input file with stored calibration matrices
	string xmlInput = "..\\camera_calibration\\camera_out.xml";

	return readCalibratedInfo(xmlInput, statusMsg, map1, map2, calibratedImageSize);
}

bool Camera::calibrateCamera(string& statusMsg){
	// input file with info about calibration parameters
	string xmlInput = "..\\camera_calibration\\camera_in_config.xml";

	return calibrateCameraFromInputFile(capture, xmlInput, statusMsg, map1, map2, calibratedImageSize);
}


// funkcia pouzije nakalibrovane hodnoty na zmenu view -> view alebo view -> rview

void Camera::calibrateImage(Mat& view, Mat* rview = NULL){
	if(initialized){
		if(rview == NULL){
			remap(view, view, map1, map2, INTER_LINEAR);
		}else{
			remap(view, *rview, map1, map2, INTER_LINEAR);
		}
	}	
}

/
bool Camera::grabCaptureImage(Mat& view, bool calibrateCapturedImage){
	// ak niesu 2 nacitania tak je jeden frame pozadu, nacitava 1 stary frame dozadu (neviem preco, ale funguje to takto)
	capture.read(view);
	capture.read(view);

	if(view.empty()) return false;

	if(calibrateCapturedImage){
		calibrateImage(view);
	}

	return true;	
}


#endif