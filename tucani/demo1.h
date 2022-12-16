#ifndef DEMO1_H
#define	DEMO1_H

using namespace std;

void runDemo1(StereoCam& stereoCam, Robot* robot, RobotMotion& robotMotion, int fd) {
    //red object 2x3 cm
    MyObject redObject = MyObject();
    redObject.setColorRed();

    while (true) {
        stereoCam.percept(STEREO_SGBM);
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

                if (objDist > 240) {
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
                        //target point
                        cv::Point3i targetPoint(objMidPoint.x, objMidPoint.y, objMidPoint.z);
                        //over target point
                        cv::Point3i overTargetPoint(objMidPoint.x, 40, objMidPoint.z);

                        robotMotion.moveArm(robotMotion.baseArmPoint, overTargetPoint, robot, fd);
                        robotMotion.moveArm(overTargetPoint, targetPoint, robot, fd);
                        cv::waitKey(500);
                        robotMotion.grip(robot, fd, redObject.iWidth);
                        cv::waitKey(500);
                        robotMotion.moveArm(targetPoint, robotMotion.cargoPoint, robot, fd);
                        cv::waitKey(500);
                        robotMotion.ungrip(robot, fd);
                        cv::waitKey(500);
                        robotMotion.moveArm(robotMotion.cargoPoint, robotMotion.baseArmPoint, robot, fd);
                    }
                }
            }
        } else {
            //nenasiel sa objekt... hladame naokolo seba, tocime sa smerom doprava
            robotMotion.turnRight(robot, fd);
        }
    }
}

#endif	/* DEMO1_H */

