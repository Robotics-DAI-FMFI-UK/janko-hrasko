#ifndef ROBOT_MOTION_MODULE_H
#define	ROBOT_MOTION_MODULE_H

#include "robot_motion_helper.h"

using namespace std;

class RobotMotion {
public:
    int movingSpeed = 220;
    int movingTimeLong = 260; //in milis
    int movingTimeShort = 130; //in milis

    double minSafeDistFromArm = 50; // in mm
    double minSafeDistFromBase = 50; // in mm

    cv::Point3i baseArmPoint = cv::Point3i(0, 60, 120);
    cv::Point3i cargoPoint = cv::Point3i(60, 80, -130);

    /* ROZMERY ROBOTA */
    const Block blockBase = {
        .xDimLow = -180,
        .xDimHigh = 180,
        .yDimLow = -90,
        .yDimHigh = 30,
        .zDimLow = -230,
        .zDimHigh = 120
    };
    const Block blockArm = {
        .xDimLow = -50,
        .xDimHigh = 50,
        .yDimLow = 0,
        .yDimHigh = 400,
        .zDimLow = -50,
        .zDimHigh = 50
    };

public:
    void initArmPosition(Robot* robot, int fd);

    void grip(Robot* robot, int fd, double gripWidth);
    void ungrip(Robot* robot, int fd);
    void moveArm(cv::Point3i fromPoint, cv::Point3i toPoint, Robot* robot, int fd);

    void moveFWD(Robot* robot, int fd);
    void moveFWD(Robot* robot, int fd, int timeInMilis);
    void moveBWD(Robot* robot, int fd);
    void moveBWD(Robot* robot, int fd, int timeInMilis);
    void turnLeft(Robot* robot, int fd);
    void turnLeft(Robot* robot, int fd, int timeInMilis);
    void turnRight(Robot* robot, int fd);
    void turnRight(Robot* robot, int fd, int timeInMilis);
private:
    void stop(Robot* robot, int fd);

    bool isPointInBlock(cv::Point3i p, Block b);
    TestPathResult testPath(cv::Point3i p1, cv::Point3i p2, Block b);
    cv::Point3i alignPositionMidPoint(cv::Point3i p);
    vector<cv::Point3i> getPath(cv::Point3i& fromPoint, cv::Point3i& toPoint); //recursive function
    vector<cv::Point3i> calculatePath(cv::Point3i fromPoint, cv::Point3i toPoint);
    double getDistance(cv::Point3i p1, cv::Point3i p2);
    void slowlyMoveArm(int* prevArm, int* nextArm, int fd, int numSteps);
};

void RobotMotion::grip(Robot* robot, int fd, double gripWidth) {
    cout << "GRIP" << endl;
    // open ~ 30mm
    // closed ~ 0mm
    double degreeGrip = 30 - gripWidth + 4;
    if (degreeGrip <= 30 && degreeGrip >= 0) {
        robot->getRoboticArm()->setServo_i_degrees(0, degreeGrip); // gripper ma hodnotu 30 (zavrety) - 0 na rozsah cez 'stupne'
        sendPacket(fd, robot->getRoboticArm()->sendServoPacket('s', 0));
    } else {
        cout << "Cannot grip this object - check dimensions" << endl;
    }
}

void RobotMotion::ungrip(Robot* robot, int fd) {
    cout << "UNGRIP" << endl;
    robot->getRoboticArm()->setServo_i_degrees(0, 1); // gripper ma hodnotu 30 (zavrety) - 0 na rozsah cez 'stupne'
    sendPacket(fd, robot->getRoboticArm()->sendServoPacket('s', 0));
}

void RobotMotion::stop(Robot* robot, int fd) {
    cout << "stop ------------------------------- [float motors]" << endl;
    sendPacket(fd, robot->sendMotorPacket('j', -1));

    //+ cas na uplne zastavenie ~ 500 ms
    usleep(500000);
}

void RobotMotion::moveFWD(Robot* robot, int fd) {
    moveFWD(robot, fd, movingTimeShort);
}

void RobotMotion::moveFWD(Robot* robot, int fd, int timeInMilis) {
    cout << "moveFWD -------------------------------- timeInMilis: " << timeInMilis << endl;
    sendPacket(fd, robot->sendMotorPacket('f', -1));

    usleep(timeInMilis * 1000);
    stop(robot, fd);
}

void RobotMotion::moveBWD(Robot* robot, int fd) {
    moveBWD(robot, fd, movingTimeShort);
}

void RobotMotion::moveBWD(Robot* robot, int fd, int timeInMilis) {
    cout << "moveBWD -------------------------------- timeInMilis: " << timeInMilis << endl;
    sendPacket(fd, robot->sendMotorPacket('g', -1));

    usleep(timeInMilis * 1000);
    stop(robot, fd);
}

void RobotMotion::turnLeft(Robot* robot, int fd) {
    turnLeft(robot, fd, movingTimeLong);
}

void RobotMotion::turnLeft(Robot* robot, int fd, int timeInMilis) {
    cout << "turnLeft -------------------------------- timeInMilis: " << timeInMilis << endl;
    robot->getMotor(0)->set_motor_dir_speed(2, movingSpeed);
    robot->getMotor(1)->set_motor_dir_speed(1, movingSpeed);
    robot->getMotor(2)->set_motor_dir_speed(1, movingSpeed);
    robot->getMotor(3)->set_motor_dir_speed(2, movingSpeed);
    for (int i = 0; i < 4; i++) {
        cout << robot->sendMotorPacket('m', i);
        sendPacket(fd, robot->sendMotorPacket('m', i));
        usleep(10000); //ak tu nieje sleep tak ide len kazdy druhy motor, asi nestiha
        cout << " - ";
    }
    cout << endl;

    usleep(timeInMilis * 1000);
    stop(robot, fd);
}

void RobotMotion::turnRight(Robot* robot, int fd) {
    turnRight(robot, fd, movingTimeLong);
}

void RobotMotion::turnRight(Robot* robot, int fd, int timeInMilis) {
    cout << "turnRight -------------------------------- timeInMilis: " << timeInMilis << endl;
    robot->getMotor(0)->set_motor_dir_speed(1, movingSpeed);
    robot->getMotor(1)->set_motor_dir_speed(2, movingSpeed);
    robot->getMotor(2)->set_motor_dir_speed(2, movingSpeed);
    robot->getMotor(3)->set_motor_dir_speed(1, movingSpeed);
    for (int i = 0; i < 4; i++) {
        cout << robot->sendMotorPacket('m', i);
        sendPacket(fd, robot->sendMotorPacket('m', i));
        usleep(10000); //ak tu nieje sleep tak ide len kazdy druhy motor, asi nestiha
        cout << " - ";
    }
    cout << endl;

    usleep(timeInMilis * 1000);
    stop(robot, fd);
}

bool RobotMotion::isPointInBlock(cv::Point3i p, Block b) {
    if (p.x < b.xDimHigh && p.x > b.xDimLow && p.y < b.yDimHigh && p.y > b.yDimLow && p.z < b.zDimHigh && p.z > b.zDimLow) {
        cout << p << " is in block " << b.xDimLow << " " << b.xDimHigh << " " << b.yDimLow << " " << b.yDimHigh << " " << b.zDimLow << " " << b.zDimHigh << endl;
        return true;
    }
    return false;
}

TestPathResult RobotMotion::testPath(cv::Point3i p1, cv::Point3i p2, Block b) {
    cv::Point3i vector = p2 - p1;
    for (double i = 0.05; i < 1; i += 0.05) {
        cv::Point3i pomPoint = p1 + (vector * i);

        if (isPointInBlock(pomPoint, b)) {
            cout << "point " << pomPoint << " collision" << endl;
            return TestPathResult(false, pomPoint);
        }
    }

    return TestPathResult(true, cv::Point3i());
    ;
}

cv::Point3i RobotMotion::alignPositionMidPoint(cv::Point3i p) {
    if (isPointInBlock(p, blockBase)) {
        cout << "point is in blockBase" << endl;
        cout << p;
        p.y += abs(blockBase.yDimHigh - p.y) + minSafeDistFromBase;
        cout << " --> " << p << endl;
    }
    if (isPointInBlock(p, blockArm)) {
        cout << "point is in blockArm" << endl;
        cout << p;
        p.x += abs(blockArm.xDimHigh - p.x) + minSafeDistFromArm;
        cout << " --> " << p << endl;
    }
    
    return p;
}

vector<cv::Point3i> RobotMotion::getPath(cv::Point3i& fromPoint, cv::Point3i& toPoint) {
    fromPoint = alignPositionMidPoint(fromPoint);
    toPoint = alignPositionMidPoint(toPoint);

    cv::waitKey(100);
    vector<cv::Point3i> resultPath;

    if (getDistance(fromPoint, toPoint) < 10) {
        cout << "diff points (" << fromPoint << " " << toPoint << ") less than 10" << endl;
        return resultPath;
    }

    TestPathResult tphBase = testPath(fromPoint, toPoint, blockBase);
    TestPathResult tphArm = testPath(fromPoint, toPoint, blockArm);
    if (!tphBase.result || !tphArm.result) {
        cv::Point3i colisionPoint;
        if (!tphBase.result) {
            colisionPoint = tphBase.colisionPoint;
        } else {
            colisionPoint = tphArm.colisionPoint;
        }
        cout << "colisionPoint: " << colisionPoint << endl;
        colisionPoint = alignPositionMidPoint(colisionPoint);

        vector<cv::Point3i> prevPath = getPath(fromPoint, colisionPoint);
        resultPath.insert(resultPath.end(), prevPath.begin(), prevPath.end());

        resultPath.push_back(colisionPoint);

        vector<cv::Point3i> nextPath = getPath(colisionPoint, toPoint);
        resultPath.insert(resultPath.end(), nextPath.begin(), nextPath.end());
    }

    return resultPath;
}

vector<cv::Point3i> RobotMotion::calculatePath(cv::Point3i fromPoint, cv::Point3i toPoint) {
    vector<cv::Point3i> resultPath;
    resultPath.push_back(fromPoint);

    vector<cv::Point3i> betweenPath = getPath(fromPoint, toPoint);
    resultPath.insert(resultPath.end(), betweenPath.begin(), betweenPath.end());

    resultPath.push_back(toPoint);

    return resultPath;
}

double RobotMotion::getDistance(cv::Point3i p1, cv::Point3i p2) {
    return sqrt(pow(abs(p1.x - p2.x), 2) + pow(abs(p1.y - p2.y), 2) + pow(abs(p1.z - p2.z), 2));
}

void RobotMotion::slowlyMoveArm(int* prevArm, int* nextArm, int fd, int numSteps) {
    if (numSteps < 10) {
        //minimum steps is 10
        numSteps = 10;
    }
    //vypis
    cout << "prev values: ";
    for (int i = 0; i < 5; i++) {
        cout << prevArm[i] << " ";
    }
    cout << endl;

    cout << "next values: ";
    for (int i = 0; i < 5; i++) {
        cout << nextArm[i] << " ";
    }
    cout << endl;


    double iter[5];
    for (int i = 0; i < 5; i++) {
        //calculate iterations ~ steps
        int fromVal = prevArm[i];
        int toVal = nextArm[i];
        iter[i] = ((double) abs(fromVal - toVal)) / numSteps;
        if (iter[i] < 1) {
            iter[i] = 1;
        }
    }

    for (int itr = 0; itr < numSteps; itr++) {
        for (int i = 0; i < 5; i++) {
            int fromVal = prevArm[i];
            int toVal = nextArm[i];

            if (fromVal < toVal) {
                int actVal = prevArm[i] + itr * iter[i];
                if (actVal <= toVal) {
                    string packet;
                    packet = ".s" + numberToString(i + 1) + numberToString(actVal) + ";";
                    sendPacket(fd, packet.c_str());
                    //cout << "packet sent: " << packet << endl;
                    usleep(100); // aby stihal mikroradic spracovavat jednolive pakety
                    //usleep(4000); // aby to bolo lepsie viditelne
                }
            } else {
                int actVal = prevArm[i] - itr * iter[i];
                if (actVal >= toVal) {
                    string packet;
                    packet = ".s" + numberToString(i + 1) + numberToString(actVal) + ";";
                    sendPacket(fd, packet.c_str());
                    //cout << "packet sent: " << packet << endl;
                    usleep(100); // aby stihal mikroradic spracovavat jednolive pakety
                    //usleep(4000); // aby to bolo lepsie viditelne
                }
            }

        }
    }
    cout << endl;
}

void RobotMotion::initArmPosition(Robot* robot, int fd) {
    ungrip(robot, fd);
    int* defaultArm = robot->getServoPositions();
    cout << "initBaseArmPosition: " << robot->getRoboticArm()->find_solution_and_do_InverseKinematics(baseArmPoint.x, baseArmPoint.z, baseArmPoint.y, 75) << endl;
    int* baseArm = robot->getServoPositions();
    //arm effector is in default position immediately when robot is turned on
    cv::Point3d defaultPoint = cv::Point3d(0, 400, 0);
    int numSteps = getDistance(defaultPoint, baseArmPoint) / 5;
    slowlyMoveArm(defaultArm, baseArm, fd, numSteps);
}

void RobotMotion::moveArm(cv::Point3i fromPoint, cv::Point3i toPoint, Robot* robot, int fd) {
    cout << "Move arm from " << fromPoint << " to " << toPoint << endl;

    vector<cv::Point3i> path = calculatePath(fromPoint, toPoint);

    cout << "Path is: ";
    for (int i = 0; i < path.size(); i++) {
        cout << path[i];
    }
    cout << endl;

    //i=1 lebo v i==0 uz sme
    for (int i = 1; i < path.size(); i++) {
        int* prevArm = robot->getServoPositions();
        int* nextArm = robot->getServoPositions();

        int q2Degree = 30;
        bool inverseResult = false;
        while (!inverseResult && q2Degree <= 125) {
            inverseResult = robot->getRoboticArm()->find_solution_and_do_InverseKinematics(path[i].x, path[i].z, path[i].y, q2Degree);
            q2Degree++;
            nextArm = robot->getServoPositions();
        }
        cout << "INVERSE RESULT: " << inverseResult << endl;

        if (inverseResult) {
            int numSteps = getDistance(path[i - 1], path[i]) / 5;
            cout << "numSteps" << numSteps << endl;
            slowlyMoveArm(prevArm, nextArm, fd, numSteps);
        }
    }
}


#endif	/* ROBOARM_MOTION_MODULE_H */

