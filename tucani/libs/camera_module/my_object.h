#ifndef MY_OBJECT_H
#define	MY_OBJECT_H

class MyObject {
public:
    // default setting for Red color detection
    int iLowH = 149;
    int iLowS = 97;
    int iLowV = 0;
    int iHighH = 179;
    int iHighS = 255;
    int iHighV = 255;

    // defaultis lego cube 1.6 x 2 cm
    double iHeight = 20; // in mm
    double iWidth = 16; // in mm

public:
    MyObject();
    MyObject(int width, int height);
    MyObject(int lowH, int lowS, int lowV, int highH, int highS, int highV);
    MyObject(int lowH, int lowS, int lowV, int highH, int highS, int highV, int height, int width);

    void setLowHSV(int hue, int saturation, int value);
    void setHighHSV(int hue, int saturation, int value);
    void setDimensions(int width, int height);

    void setColorRed();
    void setColorBlue();
    void setColorGreen();
    void setColorYellow();
};

MyObject::MyObject() {
}

MyObject::MyObject(int width, int height) {
    iHeight = height;
    iWidth = width;
}

MyObject::MyObject(int lowH, int lowS, int lowV, int highH, int highS, int highV) {
    iLowH = lowH;
    iLowS = lowS;
    iLowV = lowV;
    iHighH = highH;
    iHighS = highS;
    iHighV = highV;
}

MyObject::MyObject(int lowH, int lowS, int lowV, int highH, int highS, int highV, int height, int width) {
    iLowH = lowH;
    iLowS = lowS;
    iLowV = lowV;
    iHighH = highH;
    iHighS = highS;
    iHighV = highV;

    iHeight = height;
    iWidth = width;
}

void MyObject::setLowHSV(int hue, int saturation, int value) {
    iLowH = hue;
    iLowS = saturation;
    iLowV = value;
}

void MyObject::setHighHSV(int hue, int saturation, int value) {
    iHighH = hue;
    iHighS = saturation;
    iHighV = value;
}

void MyObject::setDimensions(int width, int height) {
    iHeight = height;
    iWidth = width;
}

void MyObject::setColorRed() {
    iLowH = 149;//173;
    iLowS = 150;
    iLowV = 120;
    iHighH = 179;//7;
    iHighS = 255;
    iHighV = 255;
}

void MyObject::setColorBlue() {
    iLowH = 101;
    iLowS = 180;
    iLowV = 150;
    iHighH = 127;
    iHighS = 255;
    iHighV = 255;
}

void MyObject::setColorGreen() {
    iLowH = 42;
    iLowS = 180;
    iLowV = 0;
    iHighH = 85;
    iHighS = 255;
    iHighV = 255;
}

void MyObject::setColorYellow() {
    iLowH = 25;
    iLowS = 180;
    iLowV = 150;
    iHighH = 35;
    iHighS = 255;
    iHighV = 255;
}

#endif	/* MY_OBJECT_H */

