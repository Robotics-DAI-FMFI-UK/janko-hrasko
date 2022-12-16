#include "libs/robot_libs.h"
#include "demo1.h"
#include "demo2.h"
#include "demo3.h"

using namespace std;

static string port = "/dev/ttyUSB0";
//static string port = "/dev/ttyO0";

int main(int argc, char *argv[]) {
    if (argc == 1) {
        cout << "Missing argument, enter one of folowing numbers:" << endl;
        cout << "1 - demo1: searching all red cubes" << endl;
        cout << "2 - demo2: serch red cube and put it on the blue cube" << endl;
        cout << "3 - demo3: switch three cubes (red, green, blue)" << endl;
        return 0;
    }

    StereoCam stereoCam = StereoCam();
    stereoCam.initConnection();

    Robot* robot = new Robot();
    int fd = robot->initConnection(port);

    RobotMotion robotMotion = RobotMotion();
    robotMotion.initArmPosition(robot, fd); //set basic position of robotic arm

   /* for (int i = -180; i <= 17; i++) {
        robot.getRoboticArm().setServo_i_degrees(4, i);
        sendPacket(fd, robot.getRoboticArm().sendServoPacket('s', 4));
        cv::waitKey(100);
    }


    robot.getRoboticArm().setServo_i_degrees(4, 0);
    sendPacket(fd, robot.getRoboticArm().sendServoPacket('s', 4));*/

    switch (atoi(argv[1])) {
         case 1:
             runDemo1(stereoCam, robot, robotMotion, fd);
             break;
         case 2:
             runDemo2(stereoCam, robot, robotMotion, fd);
             break;
         case 3:
             runDemo3(stereoCam, robot, robotMotion, fd);
             break;
         default:
             cout << "Invalid choice." << endl;
     }
}