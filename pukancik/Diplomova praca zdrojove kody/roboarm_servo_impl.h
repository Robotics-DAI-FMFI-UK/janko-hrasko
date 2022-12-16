#ifndef _PP_ROBOARM_SERVO_IMPl_H
#define _PP_ROBOARM_SERVO_IMPl_H

#ifndef _PP_ROBOARM_SERVO_H
#include "roboarm_servo.h"
#endif

using namespace std;
using namespace cv;

//--------------------------------------------------------------------------------------------------------------

double radsToDegrees(double rads);
double degreesToRads(double degrees);
double trunc(double value, int decimal_places);

//--------------------------------------------------------------------------------------------------------------

Servo::Servo(double mid, int svmid, double mxd, int svmxd) : min_degrees(mid), signal_value_for_min_deg(svmid), max_degrees(mxd), signal_value_for_max_deg(svmxd){
  rozpatie = max_degrees - min_degrees;

  akt_signal_value = signal_value_for_min_deg;
}

// spravny vysledok len ak hodnoty su v prislusnom rozpati
const double Servo::conversion_signal_value_to_degrees(int value){
  return  min_degrees + (abs(signal_value_for_min_deg - value) * rozpatie) / (double)(abs(signal_value_for_min_deg - signal_value_for_max_deg));
}

const int Servo::conversion_degrees_to_signal_value(double degrees){
	double temp = ((degrees - min_degrees)*(double)abs(signal_value_for_min_deg - signal_value_for_max_deg)) / (double)rozpatie;
	return (signal_value_for_min_deg < signal_value_for_max_deg) ? (int)floor((temp + signal_value_for_min_deg) + 0.5) : (int)floor((signal_value_for_min_deg - temp) + 0.5);
}

const bool Servo::isInRange_signal_value(int value){
  if((value < signal_value_for_min_deg)&&(value < signal_value_for_max_deg)){
    return false;
  }
  if((value > signal_value_for_min_deg)&&(value > signal_value_for_max_deg)){
    return false;
  }
  return true;
}

const bool Servo::isInRange_degrees(double degrees){
  if((degrees < min_degrees)||(degrees > max_degrees)){
    return false;
  }
  return true;
}

bool Servo::set_servo_by_signal_value(int nValue){
  if(isInRange_signal_value(nValue)){
    akt_signal_value = nValue;
    return true;
  }
  return false;
}

bool Servo::set_servo_by_degrees(double nDegrees){
  if(isInRange_degrees(nDegrees)){
    akt_signal_value = conversion_degrees_to_signal_value(nDegrees);
    return true;
  }
  return false;
}

const double Servo::getAngle_degrees(){
  return conversion_signal_value_to_degrees(akt_signal_value);
}

const double Servo::getAngle_radians(){
  return degreesToRads(getAngle_degrees());
}

//--------------------------------------------------------------------------------------------------------------

#define PI 3.1415926535897932384626433832795 

double radsToDegrees(double rads){
   return (180*rads)/PI;
}

double degreesToRads(double degrees){
   return (degrees*PI)/(double)180;
}

double trunc(double value, int decimal_places){
  double integer = 0, fractional = 0, output = 0;
  int j = 0, places = 1;

  fractional = modf(value, &output);
  for(int i = 0; i < decimal_places + 1; i++){
    fractional = modf(fractional, &integer);
    for(j = 0; j < i; j++){
      places *= 10;
    }
    output += integer / places;
    places = 1;
    fractional *= 10;
  }
  return output;
}

#endif