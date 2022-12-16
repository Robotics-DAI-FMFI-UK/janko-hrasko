#ifndef CAMERA_MODULE_H
#define	CAMERA_MODULE_H

#include "stereo_cam_helper.h"
#include "my_object.h"

const int OBJECT_TOO_CLOSE = -1;
const int OBJECT_TOO_FAR = -2;

const int STEREO_BM = 101;
const int STEREO_SGBM = 102;

struct StereoParameters {
    int preFilterSize = 13;
    int preFilterCap = 28;
    int SADWindowSize = 9;
    int minDisparity = 25;
    int numberOfDisparities = 256;
    int textureThreshold = 314;
    int uniquenessRatio = 2;
    int speckleWindowSize = 100;
    int speckleRange = 257;
    int P1 = 8 * 1 * SADWindowSize * SADWindowSize;
    int P2 = 32 * 1 * SADWindowSize * SADWindowSize;
    int disp12MaxDiff = 1;
    bool fullDP = false; // is false by default
};

class StereoCam {
public:
    bool hasGraphicInterface = true;
    StereoParameters stereoParam;
    int minObjDimension = 10; //px x px in view

    double cameraWidthAngle = 70; //in degrees
    double distanceBetweenCameras = 51; //in mm
    double cameraDistanceFromRoboticBase = 90; //in mm (+7 mm pridane aby sa nacahoval zhruba do stredu objektu, nie na zaciatok), je to od stredu zakladne ramena po sosovku kamier
    double floorLevel = -88; // floor distance from robotick base

    string urlLeftCam = "http://169.254.0.10:10001/robot.jpg";
    string urlRightCam = "http://169.254.0.10:10002/robot.jpg";

private:
    cv::Mat mx1;
    cv::Mat my1;
    cv::Mat mx2;
    cv::Mat my2;
    cv::Mat Q;

    cv::Mat img_l_color;
    cv::Mat img_r_color;

    cv::Mat img_l_rectified_color;
    cv::Mat img_r_rectified_color;

    cv::Mat img_l;
    cv::Mat img_r;

    cv::Size imageSize;
    cv::Mat vdisp;

    double objDist = OBJECT_TOO_CLOSE;
    double objX = 0;

public:
    StereoCam();
    StereoCam(bool hasGUI);

    void initConnection();
    void percept(int stereoAlgType);

    bool isObjectInView(MyObject& object);
    cv::Point3i getObjectPositionXYZ(MyObject& object);
    cv::Rect getObjectRect(MyObject& object);

    double getObjectDistance(MyObject& object);
    double getObjectX(MyObject& object);

private:
    bool downloadImage(string imgName, string url);
    void loadCalibrationXML();
    cv::Point3i recognizeObject(MyObject& object);
    cv::Mat getThresholdedImg(cv::Mat& sourceImg, MyObject& object);
    double calculateDepth(cv::Rect& rect, cv::Mat& srcImage);
    double calculateX(cv::Rect& rect, double distanceInMm);
    double cameraDistToArmDist(double cameraDistVal);
    vector<cv::Rect> findObjects(cv::Mat& sourceImg, int minDimension, MyObject& object);
    cv::Rect getLargestRect(vector<cv::Rect>& arrRect);
};

StereoCam::StereoCam() {
    loadCalibrationXML();
}

StereoCam::StereoCam(bool hasGUI) {
    loadCalibrationXML();
    hasGraphicInterface = hasGUI;
}

void StereoCam::initConnection() {
    CURL *curl;
    CURLcode res;

    //if (curl) {
    cout << "Setting cameras...";
    // set RESOLUTION 640x320
    // left cam
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://169.254.0.10:10001/robot.cgi?c");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, NULL);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        cout << "Failure: " << curl_easy_strerror(res) << endl;
    curl_easy_cleanup(curl);

    //right cam
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://169.254.0.10:10002/robot.cgi?c");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, NULL);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        cout << "Failure: " << curl_easy_strerror(res) << endl;
    curl_easy_cleanup(curl);
    //} else {
    //    cout << "Curl error!" << endl;
    //}
}

void StereoCam::loadCalibrationXML() {
    cv::FileStorage mx1_fs("xml/mx1.xml", cv::FileStorage::READ);
    cv::FileStorage my1_fs("xml/my1.xml", cv::FileStorage::READ);
    cv::FileStorage mx2_fs("xml/mx2.xml", cv::FileStorage::READ);
    cv::FileStorage my2_fs("xml/my2.xml", cv::FileStorage::READ);
    cv::FileStorage Q_fs("xml/Q.xml", cv::FileStorage::READ);

    mx1_fs["mx1"] >> mx1;
    my1_fs["my1"] >> my1;
    mx2_fs["mx2"] >> mx2;
    my2_fs["my2"] >> my2;
    Q_fs["Q"] >> Q;
}

bool StereoCam::downloadImage(string imgName, string url) {
    FILE *fp;
    CURL *curl;
    CURLcode res;

    fp = fopen(imgName.c_str(), "wb");
    if (fp == NULL) {
        cout << "File " << imgName << " cannot be opened";
        return false;
    }
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);
    if (res) {
        cout << "Cannot grab the image!\n";
        return false;
    }
    curl_easy_cleanup(curl);
    fclose(fp);

    return true;
}

void StereoCam::percept(int stereoAlgType) {
    long startTime, endTime;

    cout << "percept...";

    //check if correct type was set
    if (stereoAlgType != STEREO_BM && stereoAlgType != STEREO_SGBM) {
        cout << "Incorrect stereo algorithm type!";
        return;
    }

    startTime = getTime();
    // Download images
    if (downloadImage("left.jpg", urlLeftCam) && downloadImage("right.jpg", urlRightCam)) {
        //if successful download
        cout << "ok, images saved" << endl;

        endTime = getTime();
        cout << "calculation time of download images: " << endTime - startTime << endl;


        img_l_color = cv::imread("left.jpg", cv::IMREAD_COLOR);
        img_r_color = cv::imread("right.jpg", cv::IMREAD_COLOR);

        img_l = cv::imread("left.jpg", cv::IMREAD_GRAYSCALE);
        img_r = cv::imread("right.jpg", cv::IMREAD_GRAYSCALE);

        imageSize = cv::Size(img_l.cols, img_l.rows);

        cv::Mat disp = cv::Mat::zeros(imageSize.height, imageSize.width, CV_16S);
        vdisp = cv::Mat::zeros(imageSize.height, imageSize.width, CV_8U);
        img_l_rectified_color = cv::Mat::zeros(imageSize.height, imageSize.width, CV_8UC3);
        img_r_rectified_color = cv::Mat::zeros(imageSize.height, imageSize.width, CV_8UC3);
        cv::Mat img_l_rectified = cv::Mat::zeros(imageSize.height, imageSize.width, CV_8U);
        cv::Mat img_r_rectified = cv::Mat::zeros(imageSize.height, imageSize.width, CV_8U);

        cv::remap(img_l, img_l_rectified, mx1, my1, cv::INTER_LINEAR);
        cv::remap(img_r, img_r_rectified, mx2, my2, cv::INTER_LINEAR);

        myCvRemap(img_l_color, img_l_rectified_color, mx1, my1);
        myCvRemap(img_r_color, img_r_rectified_color, mx2, my2);

        startTime = getTime();
        if (stereoAlgType == STEREO_BM) {
            //FIND CORRESPONDENCE STEREO_BM
            cv::StereoBM bm = cv::StereoBM();
            bm.state->preFilterCap = stereoParam.preFilterCap;
            bm.state->preFilterSize = stereoParam.preFilterSize;
            bm.state->SADWindowSize = stereoParam.SADWindowSize;
            bm.state->minDisparity = stereoParam.minDisparity;
            bm.state->numberOfDisparities = stereoParam.numberOfDisparities;
            bm.state->textureThreshold = stereoParam.textureThreshold;
            bm.state->uniquenessRatio = stereoParam.uniquenessRatio;
            bm.state->speckleWindowSize = stereoParam.speckleWindowSize;
            bm.state->speckleRange = stereoParam.speckleRange;

            bm(img_l_rectified, img_r_rectified, disp);
        } else if (stereoAlgType == STEREO_SGBM) {
            //FIND CORRESPONDENCE STEREO_SGBM
            cv::StereoSGBM sgbm = cv::StereoSGBM();
            sgbm.minDisparity = stereoParam.minDisparity;
            sgbm.numberOfDisparities = stereoParam.numberOfDisparities;
            sgbm.SADWindowSize = stereoParam.SADWindowSize;
            sgbm.P1 = stereoParam.P1;
            sgbm.P2 = stereoParam.P2;
            sgbm.disp12MaxDiff = stereoParam.disp12MaxDiff;
            sgbm.preFilterCap = stereoParam.preFilterCap;
            sgbm.uniquenessRatio = stereoParam.uniquenessRatio;
            sgbm.speckleWindowSize = stereoParam.speckleWindowSize;
            sgbm.speckleRange = stereoParam.speckleRange;
            sgbm.fullDP = stereoParam.fullDP;

            sgbm(img_l_rectified, img_r_rectified, disp);
        }

        cv::normalize(disp, vdisp, 0, 255, CV_MINMAX, CV_8U);

        endTime = getTime();
        cout << "calculation time of stereo processing: " << endTime - startTime << endl;

        if (hasGraphicInterface) {
            // GUI Window for testing
            cv::imwrite("img_l_rectified_color.png", img_l_rectified_color);
            cv::imwrite("img_r_rectified_color.png", img_r_rectified_color);

            cv::imwrite("disparity.png", vdisp);

            cv::imshow("disparity", vdisp);
            cv::waitKey(100);
        }
    }
}

cv::Point3i StereoCam::recognizeObject(MyObject& object) {
    vector<cv::Rect> boundRect = findObjects(img_l_rectified_color, minObjDimension, object);

    if (boundRect.size() > 0) {
        cout << "Object found in view!" << endl;

        cv::Rect largestRect;
        largestRect = getLargestRect(boundRect);
        cout << largestRect << endl;

        cout << "calculating depth...";
        double depth = calculateDepth(largestRect, vdisp);
        cout << "done" << endl;

        if (hasGraphicInterface) {
            // GUI Windows for testing
            cv::imshow("disparity 2", vdisp);
            cv::waitKey(100);

            cv::Mat marker = cv::Mat::zeros(imageSize, CV_8UC3);
            cv::rectangle(marker, largestRect.tl(), largestRect.br(), cv::Scalar(255, 255, 255), 2, 8, 0);

            cv::imshow("marked position", img_l_rectified_color + marker);
            cv::waitKey(100);
        }

        cv::Point3i objMidPoint;
        objMidPoint.x = calculateX(largestRect, depth);
        objMidPoint.y = floorLevel + object.iHeight / 2;
        objMidPoint.z = cameraDistToArmDist(depth);
        return objMidPoint;
    }

    if (hasGraphicInterface) {
        // GUI Windows for testing
        cv::imshow("disparity 2", vdisp);
        cv::waitKey(100);

        cv::imshow("marked position", img_l_rectified_color);
        cv::waitKey(100);
    }


    return cv::Point3i(0, 0, 0);
}

bool StereoCam::isObjectInView(MyObject& object) {
    cv::Point3i objMidPoint = recognizeObject(object);

    if (objMidPoint == cv::Point3i(0, 0, 0)) {
        return false;
    }
    return true;
}

cv::Point3i StereoCam::getObjectPositionXYZ(MyObject& object) {
    return recognizeObject(object);
}

double StereoCam::getObjectDistance(MyObject& object) {
    return recognizeObject(object).z;
}

double StereoCam::getObjectX(MyObject& object) {
    return recognizeObject(object).x;
}

vector<cv::Rect> StereoCam::findObjects(cv::Mat& sourceImg, int minDimension, MyObject& object) {
    vector<cv::Rect> resultBoundRectArray;

    cv::Mat imgThresholded = getThresholdedImg(sourceImg, object);

    //get contours
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(imgThresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    vector<vector<cv::Point> > contours_poly(contours.size());

    for (int i = 0; i < contours.size(); i++) {
        cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
        cv::Rect rect = cv::boundingRect(cv::Mat(contours_poly[i]));

        if (rect.width > minDimension && rect.height > minDimension) {
            resultBoundRectArray.push_back(rect);
        }
    }

    return resultBoundRectArray;
}

cv::Rect StereoCam::getLargestRect(vector<cv::Rect>& arrRect) {
    cout << "start getLargestRect" << endl;
    cout << "size: " << arrRect.size() << endl;

    cv::Rect resultRect;
    if (arrRect.size() > 0) {
        resultRect = arrRect[0];
    }

    cout << "half getLargestRect" << endl;

    //hladame najvacsi obdlznik podla plochy
    for (int i = 0; i < arrRect.size(); i++) {
        if (arrRect[i].area() > resultRect.area()) {
            resultRect = arrRect[i];
        }
    }
    cout << "end getLargestRect" << endl;
    cout << resultRect << endl;
    return resultRect;
}

double StereoCam::calculateDepth(cv::Rect& rect, cv::Mat& disparityMap) {
    //histogram napleny nulovymi hodnotami
    int histogram[1000] = {0};

    int yStart = rect.tl().y;
    int yEnd = rect.br().y;
    int xStart = rect.tl().x;
    int xEnd = rect.br().x;

    //try enlarge area by 3px from all sides
    if (yStart - 3 > 0) {
        yStart -= 3;
    }
    if (yEnd + 3 < disparityMap.rows) {
        yStart += 3;
    }
    if (xStart - 3 > 0) {
        xStart -= 3;
    }
    if (xEnd + 3 < disparityMap.cols) {
        xEnd += 3;
    }

    //reproject disparity to 3D    
    cv::Mat recons3D(imageSize, CV_32FC3);
    cv::reprojectImageTo3D(disparityMap, recons3D, Q, false, CV_32F);

    //make histogram
    for (int y = yStart; y <= yEnd; y++) {
        float* recons_ptr = recons3D.ptr<float>(y);
        for (int x = xStart; x <= xEnd; x++) {
            double valZ = recons_ptr[3 * x + 2];
            if (valZ > 0) {
                histogram[(int) valZ]++;
            }
        }
    }
    //zisti dominantnu hodnotu Z (int)
    int dominantValZ = max_hist(histogram, 1000);

    double sum = 0;
    int count = 0;

    for (int y = yStart; y <= yEnd; y++) {
        float* recons_ptr = recons3D.ptr<float>(y);
        for (int x = xStart; x <= xEnd; x++) {
            cv::Point3i point;
            point.x = recons_ptr[3 * x];
            point.y = recons_ptr[3 * x + 1];
            point.z = recons_ptr[3 * x + 2];

            // get pixel Z value
            double valZ = point.z;

            if (valZ > 0) {
                if (abs(valZ - dominantValZ) < 3) {
                    count++;
                    sum += valZ;
                }
            } else {
                //example of set pixel
                disparityMap.at<uchar>(cv::Point(x, y)) = 255;
            }
        }
    }

    if (count == 0) {
        return 0;
    }

    // calculate weighted average
    // *10 to mm
    return sum / count * 10;
}

double StereoCam::calculateX(cv::Rect& rect, double distanceInMm) {
    //calculation of X position is from left camera
    double resultX = 0;

    //left camera -> shift is negative
    double shiftFromX0 = -(distanceBetweenCameras / 2);
    cv::Point rectMiddle;
    rectMiddle.x = rect.tl().x + rect.width / 2;
    rectMiddle.y = rect.tl().y + rect.height / 2;
    double alpha = ((double) rectMiddle.x / imageSize.width) * cameraWidthAngle - (cameraWidthAngle / 2);
    resultX = tan(alpha * M_PI / 180) * distanceInMm;

    return resultX + shiftFromX0;
}

double StereoCam::cameraDistToArmDist(double cameraDistVal) {
    // vypocitana vzdialenost od kamier
    double result = cameraDistVal + cameraDistanceFromRoboticBase;

    if (cameraDistVal < 120) {
        result = OBJECT_TOO_CLOSE;
        cout << "object too close" << endl;
    } else if (cameraDistVal > 300) {
        result = OBJECT_TOO_FAR;
        cout << "object too far" << endl;
    }

    return result;
}

cv::Mat StereoCam::getThresholdedImg(cv::Mat& sourceImg, MyObject& object) {
    cv::Mat imgHSV;

    cv::cvtColor(sourceImg, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

    cv::Mat imgThresh;

    cv::inRange(imgHSV, cv::Scalar(object.iLowH, object.iLowS, object.iLowV), cv::Scalar(object.iHighH, object.iHighS, object.iHighV), imgThresh); //Threshold the image

    //morphological opening (removes small objects from the foreground)
    cv::erode(imgThresh, imgThresh, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::dilate(imgThresh, imgThresh, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

    //morphological closing (removes small holes from the foreground)
    cv::dilate(imgThresh, imgThresh, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::erode(imgThresh, imgThresh, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

    if (hasGraphicInterface) {
        // GUI Window for testing
        cv::imshow("thresholded", imgThresh);
        cv::waitKey(100);
    }

    return imgThresh;
}

#endif	/* CAMERA_MODULE_H */

