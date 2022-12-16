#ifndef DEMO3_H
#define	DEMO3_H

#include "libs/camera_module/my_object.h"
#include "libs/camera_module/stereo_cam.h"
#include "libs/robot_motion_module/robot_motion.h"


using namespace std;


const int RED = 101;
const int GREEN = 102;
const int BLUE = 103;
const int FREE = 100;

class WorldState {
public:
    int positions[4];
    vector<int> actions;

public:

    WorldState() {
    };

    WorldState(int pos1, int pos2, int pos3, int pos4) {
        positions[0] = pos1;
        positions[1] = pos2;
        positions[2] = pos3;
        positions[3] = pos4;
    }

    WorldState(int pos1, int pos2, int pos3, int pos4, vector<int> prevActions) {
        positions[0] = pos1;
        positions[1] = pos2;
        positions[2] = pos3;
        positions[3] = pos4;
        actions = prevActions;
    }

    bool operator==(WorldState ws) {
        if (positions[0] != ws.positions[0] || positions[1] != ws.positions[1] || positions[2] != ws.positions[2] || positions[3] != ws.positions[3]) {
            return false;
        }
        return true;
    };
};

WorldState actionMove(int index, WorldState prevWS) {
    //find free space position
    int freePos;
    for (int i = 0; i < 4; i++) {
        if (prevWS.positions[i] == FREE) {
            freePos = i;
            break;
        }
    }

    //make new world state where are positions switched
    vector<int> prevActions = prevWS.actions;
    prevActions.push_back(index);
    WorldState resultWS = WorldState(prevWS.positions[0], prevWS.positions[1], prevWS.positions[2], prevWS.positions[3], prevActions);
    int tmp = resultWS.positions[index];
    resultWS.positions[index] = resultWS.positions[freePos];
    resultWS.positions[freePos] = tmp;

    return resultWS;
}

vector<int> getActions(WorldState startWS, WorldState finalWS) {
    vector<WorldState> stateArray;
    stateArray.push_back(startWS);

    vector<WorldState> workArray;
    while (stateArray.size() > 0) {
        WorldState actWS = stateArray.back();
        stateArray.pop_back();

        if (actWS == finalWS) {
            //mame vitaza
            cout << "found final world state" << endl;
            return actWS.actions;
        } else {
            //generate all posible states - except FREE space
            for (int i = 0; i < 4; i++) {
                if (actWS.positions[i] != FREE) {
                    workArray.push_back(actionMove(i, actWS));
                }
            }
            if (stateArray.size() == 0) {
                stateArray = workArray;
                vector<WorldState> newWorkArray;
                workArray = newWorkArray;
            }
        }
    }
}

void move(cv::Point3i fromPoint, MyObject obj, cv::Point3i toPoint, StereoCam& stereoCam, Robot* robot, RobotMotion& robotMotion, int fd) {
    bool objPickedUp = false;
    while (!objPickedUp) {
        robotMotion.ungrip(robot, fd);

        stereoCam.percept(STEREO_SGBM);

        cv::Point3i overFromPoint(fromPoint.x, fromPoint.y + 40, fromPoint.z);
        robotMotion.moveArm(robotMotion.baseArmPoint, overFromPoint, robot, fd);
        robotMotion.moveArm(overFromPoint, fromPoint, robot, fd);
        cv::waitKey(500);
        robotMotion.grip(robot, fd, obj.iWidth);
        cv::waitKey(500);
        robotMotion.moveArm(fromPoint, robotMotion.baseArmPoint, robot, fd);

        //kontrola ci sme zdvihli objekt
        stereoCam.percept(STEREO_SGBM);
        if (!stereoCam.isObjectInView(obj)) {
            objPickedUp = true;
        } else {
            objPickedUp = false;
        }
    }

    cv::Point3i overToPoint(toPoint.x, toPoint.y + 40, toPoint.z);
    robotMotion.moveArm(robotMotion.baseArmPoint, overToPoint, robot, fd);
    robotMotion.moveArm(overToPoint, toPoint, robot, fd);
    cv::waitKey(500);
    robotMotion.ungrip(robot, fd);
    cv::waitKey(500);
    robotMotion.moveArm(toPoint, robotMotion.baseArmPoint, robot, fd);
}

void moveFromCargo(cv::Point3i fromPoint, MyObject obj, cv::Point3i toPoint, StereoCam& stereoCam, Robot* robot, RobotMotion& robotMotion, int fd) {
    robotMotion.ungrip(robot, fd);

    cv::Point3i overFromPoint(fromPoint.x, fromPoint.y + 40, fromPoint.z);
    robotMotion.moveArm(robotMotion.baseArmPoint, overFromPoint, robot, fd);
    robotMotion.moveArm(overFromPoint, fromPoint, robot, fd);
    cv::waitKey(500);
    robotMotion.grip(robot, fd, obj.iWidth);
    cv::waitKey(500);
    robotMotion.moveArm(fromPoint, robotMotion.baseArmPoint, robot, fd);

    cv::Point3i overToPoint(toPoint.x, toPoint.y + 40, toPoint.z);
    robotMotion.moveArm(robotMotion.baseArmPoint, overToPoint, robot, fd);
    robotMotion.moveArm(overToPoint, toPoint, robot, fd);
    cv::waitKey(500);
    robotMotion.ungrip(robot, fd);
    cv::waitKey(500);
    robotMotion.moveArm(toPoint, robotMotion.baseArmPoint, robot, fd);
}

void runDemo3(StereoCam& stereoCam, Robot* robot, RobotMotion& robotMotion, int fd) {
    cout << "runing demo 3" << endl;
    vector<cv::Point3i> positions3D;
    WorldState startWS;

    //init final world state
    WorldState finalWS = WorldState(RED, GREEN, BLUE, FREE);
    cout << "finalWS init: 0-" << finalWS.positions[0] << " 1-" << finalWS.positions[1] << " 2-" << finalWS.positions[2] << " 3-" << finalWS.positions[3] << endl;

    //red lego cube 
    MyObject redObject = MyObject();
    redObject.setColorRed();
    //blue lego cube
    MyObject blueObject = MyObject();
    blueObject.setColorBlue();
    //green lego cube 
    MyObject greenObject = MyObject();
    greenObject.setColorGreen();

    stereoCam.percept(STEREO_SGBM);

    cv::Point3i redObjMidPoint = stereoCam.getObjectPositionXYZ(redObject);
    cv::Point3i greenObjMidPoint = stereoCam.getObjectPositionXYZ(greenObject);
    cv::Point3i blueObjMidPoint = stereoCam.getObjectPositionXYZ(blueObject);

    cout << "RED: " << redObjMidPoint << endl;
    cout << "GREEN: " << greenObjMidPoint << endl;
    cout << "BLUE: " << blueObjMidPoint << endl;

    while (redObjMidPoint.z <= 0 || greenObjMidPoint.z <= 0 || blueObjMidPoint.z <= 0) {
        cout << redObjMidPoint.z << " <= 0 " << " || " << greenObjMidPoint.z << " <= 0 " << " || " << blueObjMidPoint.z << " <= 0 " << endl;
        stereoCam.percept(STEREO_SGBM);

        redObjMidPoint = stereoCam.getObjectPositionXYZ(redObject);
        greenObjMidPoint = stereoCam.getObjectPositionXYZ(greenObject);
        blueObjMidPoint = stereoCam.getObjectPositionXYZ(blueObject);

        cout << "RED: " << redObjMidPoint << endl;
        cout << "GREEN: " << greenObjMidPoint << endl;
        cout << "BLUE: " << blueObjMidPoint << endl;
    }

    if (redObjMidPoint.x < blueObjMidPoint.x && redObjMidPoint.x < greenObjMidPoint.x) {
        //RED is first
        positions3D.push_back(redObjMidPoint);
        if (blueObjMidPoint.x < greenObjMidPoint.x) {
            //BLUE is second
            startWS = WorldState(RED, BLUE, GREEN, FREE);
            positions3D.push_back(blueObjMidPoint);
            positions3D.push_back(greenObjMidPoint);
        } else {
            //GREEN is second
            startWS = WorldState(RED, GREEN, BLUE, FREE);
            positions3D.push_back(greenObjMidPoint);
            positions3D.push_back(blueObjMidPoint);
        }
    } else if (blueObjMidPoint.x < redObjMidPoint.x && blueObjMidPoint.x < greenObjMidPoint.x) {
        //BLUE is first
        positions3D.push_back(blueObjMidPoint);
        if (redObjMidPoint.x < greenObjMidPoint.x) {
            //RED is second
            positions3D.push_back(redObjMidPoint);
            positions3D.push_back(greenObjMidPoint);
            startWS = WorldState(BLUE, RED, GREEN, FREE);
        } else {
            //GREEN is second
            positions3D.push_back(greenObjMidPoint);
            positions3D.push_back(redObjMidPoint);
            startWS = WorldState(BLUE, GREEN, RED, FREE);
        }
    } else {
        //GREEN is first
        positions3D.push_back(greenObjMidPoint);
        if (redObjMidPoint.x < blueObjMidPoint.x) {
            //RED is second
            positions3D.push_back(redObjMidPoint);
            positions3D.push_back(blueObjMidPoint);
            startWS = WorldState(GREEN, RED, BLUE, FREE);
        } else {
            //BLUE is second
            positions3D.push_back(blueObjMidPoint);
            positions3D.push_back(redObjMidPoint);
            startWS = WorldState(GREEN, BLUE, RED, FREE);
        }
    }
    positions3D.push_back(cv::Point3i(50, 71, -120));

    //get move actions from start world state to final world state
    cout << "startWS: 0-" << startWS.positions[0] << " 1-" << startWS.positions[1] << " 2-" << startWS.positions[2] << " 3-" << startWS.positions[3] << endl;
    cout << "finalWS: 0-" << finalWS.positions[0] << " 1-" << finalWS.positions[1] << " 2-" << finalWS.positions[2] << " 3-" << finalWS.positions[3] << endl;
    cout << "searching for solution..." << endl;
    vector<int> actions = getActions(startWS, finalWS);
    cout << "solution found" << endl;


    int step = 0;
    while (step < actions.size()) {
        int actionIndex = actions[step];

        int freePosIndex;
        for (int i = 0; i < 4; i++) {
            if (startWS.positions[i] == FREE) {
                freePosIndex = i;
                break;
            }
        }

        cout << "freePosIndex is " << freePosIndex << endl;
        cout << "actionIndex is " << actionIndex << endl;
        if (actionIndex == 3) {
            //pick up object from cargo base
            moveFromCargo(positions3D[4], MyObject(), positions3D[freePosIndex], stereoCam, robot, robotMotion, fd);
        } else {
            //pick up object in front of robot
            switch (startWS.positions[actionIndex]) {
                case RED:
                    redObjMidPoint = stereoCam.getObjectPositionXYZ(redObject);
                    cout << redObjMidPoint << endl;
                    while (redObjMidPoint.z <= 0) {
                        stereoCam.percept(STEREO_SGBM);
                        redObjMidPoint = stereoCam.getObjectPositionXYZ(redObject);
                        cout << redObjMidPoint << endl;
                    }
                    move(redObjMidPoint, redObject, positions3D[freePosIndex], stereoCam, robot, robotMotion, fd);
                    break;
                case GREEN:
                    greenObjMidPoint = stereoCam.getObjectPositionXYZ(greenObject);
                    cout << greenObjMidPoint << endl;
                    while (greenObjMidPoint.z <= 0) {
                        stereoCam.percept(STEREO_SGBM);
                        greenObjMidPoint = stereoCam.getObjectPositionXYZ(greenObject);
                        cout << greenObjMidPoint << endl;
                    }
                    move(greenObjMidPoint, greenObject, positions3D[freePosIndex], stereoCam, robot, robotMotion, fd);
                    break;
                case BLUE:
                    blueObjMidPoint = stereoCam.getObjectPositionXYZ(blueObject);
                    cout << blueObjMidPoint << endl;
                    while (blueObjMidPoint.z <= 0) {
                        stereoCam.percept(STEREO_SGBM);
                        blueObjMidPoint = stereoCam.getObjectPositionXYZ(blueObject);
                        cout << blueObjMidPoint << endl;
                    }
                    move(blueObjMidPoint, blueObject, positions3D[freePosIndex], stereoCam, robot, robotMotion, fd);
                    break;
            }
        }

        int tmp = startWS.positions[actionIndex];
        startWS.positions[actionIndex] = startWS.positions[freePosIndex];
        startWS.positions[freePosIndex] = tmp;

        step++;
    }
}

#endif	/* DEMO3_H */

