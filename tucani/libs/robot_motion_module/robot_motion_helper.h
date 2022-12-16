#ifndef ROBOT_MOTION_HELPER_H
#define	ROBOT_MOTION_HELPER_H

using namespace std;

//STRUCTURES

struct Block {
    int xDimLow;
    int xDimHigh;
    int yDimLow;
    int yDimHigh;
    int zDimLow;
    int zDimHigh;
};

struct TestPathResult {
    bool result;
    cv::Point3i colisionPoint;

    TestPathResult(bool result2, cv::Point3i colisionPoint2) : result(result2), colisionPoint(colisionPoint2) {
    };
};

//FUNCTIONS

template <typename T>
string numberToString(T Number) {
    stringstream ss;
    ss << Number;
    return ss.str();
}

#endif	/* ROBOT_MOTION_HELPER_H */

