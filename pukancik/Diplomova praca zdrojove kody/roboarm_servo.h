#ifndef _PP_ROBOARM_SERVO_H
#define _PP_ROBOARM_SERVO_H

using namespace std;
using namespace cv;

//--------------------------------------------------------------------------------------------------------------

class Servo{
    
	double min_degrees;                 
    double max_degrees;
    int signal_value_for_min_deg;       // value of signal of servo motor for minimal value of possible angle range interval
    int signal_value_for_max_deg;       // value of signal of servo motor for maximal value of possible angle range interval
    double rozpatie;                    // possible angle range interval = max_degrees - min_degrees
    int akt_signal_value;               // actual value of signal of servo motor

  public:
  
	Servo(double mid, int svmid, double mxd, int svmxd);

    const double conversion_signal_value_to_degrees(int value);
	const int conversion_degrees_to_signal_value(double degrees);

    const bool isInRange_signal_value(int value);
    const bool isInRange_degrees(double degrees);

    bool set_servo_by_signal_value(int nValue);
    bool set_servo_by_degrees(double nDegrees);

    const int get_akt_signal_value(){return akt_signal_value;};
    const double getAngle_degrees();
    const double getAngle_radians();

	const double getMedianDegree(){return (max_degrees-min_degrees)/(double)2;};
};

#endif
