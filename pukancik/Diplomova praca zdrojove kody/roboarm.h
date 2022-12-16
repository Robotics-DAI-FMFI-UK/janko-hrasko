#ifndef _PP_ROBOARM_H
#define _PP_ROBOARM_H

#ifndef _PP_ROBOARM_SERVO_H
#include "roboarm_servo.h"
#endif

using namespace std;
using namespace cv;

// --------------------------------------------------------------------------------------------------------------

struct MyRoboArm_IK_Result{
  double q0, q1, q2, q3, q4, q5;
  bool reachable;
  MyRoboArm_IK_Result(bool nReachable = true, double nq0 = 0, double nq1 = 0, double nq2 = 0, double nq3 = 0, double nq4 = 0, double nq5 = 0) : reachable(nReachable), q0(nq0), q1(nq1), q2(nq2), q3(nq3), q4(nq4), q5(nq5){};
};

// --------------------------------------------------------------------------------------------------------------

class MyRoboticArm{
	Servo* servos[6];		// servos[0] is servo for base DOF of RoboArm, servos[1] ... servos[3], servos[4] is rotating servo just before gripper, servos[5] is effector (gripper)
    int links[6];			// scale is in milimeters: links[0] is height of RoboArm base, links[1] is length of link from DOF 1 to 2, ...
  

  public:
    MyRoboticArm();
    // ~MyRoboticArm(){delete [] servos;};	// blbne to

    bool setServo_i_degrees(int i, double degrees);

    const Point3d ForwardKinematics(double x = 0, double y = 0, double z = 0){return ForwardKinematics(Point3d(x,y,z));}   // bez parametra bude vstup (0,0,0)
    const Point3d ForwardKinematics(Point3d point);

    const MyRoboArm_IK_Result InverseKinematics(double x, double y, double z, double q3){return InverseKinematics(Point3d(x,y,z),q3);};
    const MyRoboArm_IK_Result InverseKinematics(Point3d point, double q3);

	// IK functionality needs prefered value for q3 (aka angle for servo[3]), this functions tries only with this angle
    bool find_solution_and_do_InverseKinematics(double x, double y, double z, double q3){return find_solution_and_do_InverseKinematics(Point3d(x,y,z),q3);};
    bool find_solution_and_do_InverseKinematics(Point3d point, double q3);

	// IK functionality needs prefered value for q3 (aka angle for servo[3]), this functions tries first time with this angle, then tries with the whole scale of servo[3] angle range
	bool find_IK_solution_with_prefered_angle_and_do_IK(double x, double y, double z, double q3){return find_IK_solution_with_prefered_angle_and_do_IK(Point3d(x,y,z),q3);};
    bool find_IK_solution_with_prefered_angle_and_do_IK(Point3d point, double q3);

	bool trySetOneServo(int id, double q);

	// functions return appropriate form of protocol command in string format
	string getOneServosPacketCommand(int index, int time);						// default value is 2 seconds (2000ms)
    string getAllServosPacketCommand(int time, bool init);						// default value is 2 seconds (2000ms) and init is false by default
};

#endif