#ifndef DEMO2_H
#define	DEMO2_H

using namespace std;

bool pickUp(cv::Point3i objMidPoint, MyObject obj, StereoCam& stereoCam, Robot* robot, RobotMotion& robotMotion, int fd) {
    //target point
    cv::Point3i targetPoint(objMidPoint.x, objMidPoint.y, objMidPoint.z);
    //over target point
    cv::Point3i overTargetPoint(objMidPoint.x, 40, objMidPoint.z);

    robotMotion.moveArm(robotMotion.baseArmPoint, overTargetPoint, robot, fd);
    robotMotion.moveArm(overTargetPoint, targetPoint, robot, fd);
    cv::waitKey(500);
    robotMotion.grip(robot, fd, obj.iWidth);
    cv::waitKey(500);
    robotMotion.moveArm(targetPoint, robotMotion.baseArmPoint, robot, fd);

    //kontrola ci sme zdvihli cerveny objekt
    stereoCam.percept(STEREO_SGBM);
    if (!stereoCam.isObjectInView(obj)) {
        return true;
    }
    return false;
}

void runDemo2(StereoCam& stereoCam, Robot* robot, RobotMotion& robotMotion, int fd) {
    //red lego cube 
    MyObject redObject = MyObject();
    redObject.setColorRed();
    //blue lego cube
    MyObject blueObject = MyObject();
    blueObject.setColorBlue();

    bool foundRedObject = false;

    while (true) {
        stereoCam.percept(STEREO_SGBM);
        if (!foundRedObject) {
            cv::Point3i objMidPoint = stereoCam.getObjectPositionXYZ(redObject);
            cout << "redObject position is: " << objMidPoint << endl;

            if (stereoCam.isObjectInView(redObject)) {
                double objDist = objMidPoint.z;

                if (objDist == OBJECT_TOO_FAR) {
                    //posunieme sa kusok dopredu
                    robotMotion.moveFWD(robot, fd);
                } else if (objDist == OBJECT_TOO_CLOSE) {
                    //posunieme sa kusok dozadu
                    robotMotion.moveBWD(robot, fd);
                } else {
                    cout << "objDist is " << objDist << endl;

                    double objX = objMidPoint.x;
                    cout << "objX is " << objX << endl;

                    if (objDist > 250) {
                        // stale je objekt out of reach
                        if (objX < -40) {
                            //objekt je prilis vlavo
                            robotMotion.turnLeft(robot, fd);
                        } else if (objX > 40) {
                            //objekt je prilis vpravo
                            robotMotion.turnRight(robot, fd);
                        } else {
                            //je pekne pred nami, ale este prilis vzdialeny
                            robotMotion.moveFWD(robot, fd);
                        }
                    } else {
                        //objekt je v dosahu - vzdialenost
                        if (objX < -40) {
                            //objekt je prilis vlavo
                            robotMotion.turnLeft(robot, fd);
                        } else if (objX > 40) {
                            //objekt je prilis vpravo
                            robotMotion.turnRight(robot, fd);
                        } else {
                            //objekt je pred nami - v dosahu ramena 
                            foundRedObject = pickUp(objMidPoint, redObject, stereoCam, robot, robotMotion, fd);
                        }
                    }
                }
            } else {
                //nenasiel sa objekt... hladame naokolo seba, tocime sa smerom doprava
                robotMotion.turnRight(robot, fd);
            }
        } else {
            //if redObject is found & gripped, searching for blueObject
            cv::Point3i objMidPoint = stereoCam.getObjectPositionXYZ(blueObject);
            cout << "redObject position is: " << objMidPoint << endl;

            if (stereoCam.isObjectInView(blueObject)) {
                double objDist = objMidPoint.z;

                if (objDist == OBJECT_TOO_FAR) {
                    //posunieme sa kusok dopredu
                    robotMotion.moveFWD(robot, fd);
                } else if (objDist == OBJECT_TOO_CLOSE) {
                    //posunieme sa kusok dozadu
                    robotMotion.moveBWD(robot, fd);
                } else {
                    cout << "objDist is " << objDist << endl;

                    double objX = objMidPoint.x;
                    cout << "objX is " << objX << endl;

                    if (objDist > 250) {
                        // stale je objekt out of reach
                        if (objX < -40) {
                            //objekt je prilis vlavo
                            robotMotion.turnLeft(robot, fd);
                        } else if (objX > 40) {
                            //objekt je prilis vpravo
                            robotMotion.turnRight(robot, fd);
                        } else {
                            //je pekne pred nami, ale este prilis vzdialeny
                            robotMotion.moveFWD(robot, fd);
                        }
                    } else {
                        //objekt je v dosahu - vzdialenost
                        if (objX < -40) {
                            //objekt je prilis vlavo
                            robotMotion.turnLeft(robot, fd);
                        } else if (objX > 40) {
                            //objekt je prilis vpravo
                            robotMotion.turnRight(robot, fd);
                        } else {
                            //objekt je pred nami - v dosahu ramena 

                            //target point + add height of blueObject
                            cv::Point3i targetPoint(objMidPoint.x, objMidPoint.y + blueObject.iHeight, objMidPoint.z);
                            //over target point
                            cv::Point3i overTargetPoint(objMidPoint.x, 40, objMidPoint.z);

                            robotMotion.moveArm(robotMotion.baseArmPoint, overTargetPoint, robot, fd);
                            robotMotion.moveArm(overTargetPoint, targetPoint, robot, fd);
                            cv::waitKey(1000);
                            robotMotion.ungrip(robot, fd);
                            cv::waitKey(500);
                            robotMotion.moveArm(targetPoint, overTargetPoint, robot, fd);
                            robotMotion.moveArm(overTargetPoint, robotMotion.baseArmPoint, robot, fd);

                            break;
                        }
                    }
                }
            } else {
                //nenasiel sa objekt... hladame naokolo seba, tocime sa smerom doprava
                robotMotion.turnRight(robot, fd);
            }
        }
    }
}


#endif	/* DEMO2_H */

