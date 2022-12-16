#ifndef _PP_ROBOARM_IMPl_H
#define _PP_ROBOARM_IMPl_H

#ifndef _PP_ROBOARM_H
#include "roboarm.h"
#endif

using namespace std;
using namespace cv;

// -------------------------------------------------------------------------------------------------------------------

MyRoboticArm::MyRoboticArm(){
  servos[0] = new Servo(0,2370,180,600);           
  servos[1] = new Servo(19,752,140,1900);
  servos[2] = new Servo(0,925,136,2252);
  servos[3] = new Servo(0,1480,100,550);
  servos[4] = new Servo(0,1555,90,645);			// 0 je ak je Gripper z pohladu "zozadu" v tvare |_|, 90 je otoceny tak ze je kolmo cize -|
  servos[5] = new Servo(7,2100,31,1400);		// Gripper(aj End Effector) funguje rovnako ako rotation serva, len namiesto uhlov pracuje so sirkou chapadla, v tomto pripade 31 mm su najsirsie, 7 su najviac ako ide zatvorene
												
  // scale is in milimeters: links[0] is height of RoboArm base, links[1] is length of link from DOF 1 to 2, ...
  links[0] = 48;
  links[1] = 19;
  links[2] = 147;
  links[3] = 182;
  links[4] = 56;
  links[5] = 58;	// toto je dlzka az po koniec efektora, nie len toho posledneho linku
}

bool MyRoboticArm::setServo_i_degrees(int i, double degrees){
  if((i<0)||(i>5)){
    return false;
  }
  return servos[i]->set_servo_by_degrees(degrees);  
}

// bez parametra bude vstup MyPoint() => (0,0,0)
const Point3d MyRoboticArm::ForwardKinematics(Point3d point){
  double q0 = servos[0]->getAngle_radians();
  double q1 = servos[1]->getAngle_radians();
  double q2 = servos[2]->getAngle_radians();
  double q3 = servos[3]->getAngle_radians();
  // q4 a q5 nepotrebujem

  double d1 = links[0];
  double d2 = links[1];
  double a3 = links[2];
  double a4 = links[3];
  double a5 = links[4];
		a5 += links[5];		// este musim priratat, aby to bolo az ku koncu grippera, ten DOF co je medzi 4 a 5 nieni zatial podstatny
  
  double x = 0, y = 0, z = 0;
  if(!((abs(point.x) < 0.00001)&&(abs(point.y) < 0.00001)&&(abs(point.z) < 0.00001))){
    x =	cos(q0)*cos(q1-q2-q3)*point.x +	cos(q0)*sin(q1-q2-q3)*point.y + (-1)*sin(q0)*point.z;
    y = sin(q0)*cos(q1-q2-q3)*point.x + sin(q0)*sin(q1-q2-q3)*point.y +		 cos(q0)*point.z;
    z = sin(q1-q2-q3)*point.x		  +	   (-1)*cos(q1-q2-q3)*point.y;
  }

  double temp = a5*cos(q1-q2-q3) + a4*cos(q1-q2) + a3*cos(q1);
  x+= cos(q0)*temp;
  y+= sin(q0)*temp;
  z+= a5*sin(q1-q2-q3) + a4*sin(q1-q2) + a3*sin(q1) + d1 + d2;

  // odstranit tesne nuly
  if(abs(x) < 0.00001) x = (double)0;
  if(abs(y) < 0.00001) y = (double)0;
  if(abs(z) < 0.00001) z = (double)0;

  return Point3d(x,y,z);
}

// IK je urobene tak, ze sa urci preferovana hodnota pre q3, cize servo[3]
const MyRoboArm_IK_Result MyRoboticArm::InverseKinematics(Point3d point, double q3){
	// premenne
	double d1 = links[0];
	double d2 = links[1];
	double a3 = links[2];
	double a4 = links[3];
	double a5 = links[4];
		  a5 += links[5];		// este musim priratat, aby to bolo az ku koncu grippera, ten DOF co je medzi 4 a 5 neni zatial podstatny

	// vypocet - prva cast
	double tZ = point.z - d1 - d2;										// bod v transformovanej (pomocnej) rovine, je to vyska hladaneho bodu ramena v povodnom surad. systeme na osiZ, ale az od suradn. systemu A2
	double tXY = 0;														// bod v transformovanej (pomocnej) rovine, je to dlzka ramena v povodnej rovine osX a osY

	double q0 = servos[0]->getAngle_degrees();							// zatial sa servu_0 (zakladna) nastavi uhol co tam je aktualne
  
	if((point.x < 0)||(point.x > 0)||(point.y < 0)||(point.y > 0)){		// ak su obe rovne nule, atan2 vrati chybu. v takomto pripade je servo[0] lubovolne natocene, preto nech zostane to ako je aktualne a tXY zostane 0
		double temp_atanXY = atan2(point.y,point.x);					// normalne sa zadava atan2(y,x)
	    q0 = radsToDegrees(temp_atanXY);
		
		if(!(servos[0]->isInRange_degrees(q0))){						// test ci sa vie do smerom k tomu bodu zakladna (servo_0) vobec natocit
			return MyRoboArm_IK_Result(false);
		}

		if(abs(temp_atanXY - (PI/(double)2)) < 0.00001){				// inak povedane ak sa realne cisla skoro rovnaju
		// nemozem delit cosinus, lebo cos 90 == 0
			tXY = point.y / sin(temp_atanXY);
		}else{
			tXY = point.x / cos(temp_atanXY);
		}
	}

	// vypocet - druha cast, najskor sa vyrata uhol q1
	double temp_angle1_d = radsToDegrees(atan2(tZ,tXY));
	if(abs(temp_angle1_d) < 0.001) temp_angle1_d = (double)0.0;					// mozem zanedbat take male cislo

	double mPow2 = (a4*a4) + (a5*a5) - 2*a4*a5*cos(degreesToRads(180 - q3));	// cosinusova veta
	double bPow2 = (tXY*tXY) + (tZ*tZ);
	
	//---
	double temp2 = (mPow2 - (a3*a3) - bPow2)/((-2)*a3*sqrt(bPow2));				// cosinusova veta, temp2 je cos(uhla) oproti mPow2 v trojuholniku sqrt(bPow2)---sqrt(mPow2)---a3, a spolu s temp_angle1_d tvoria sucet, ktory po odratani od 90 dostanem q1

	if(temp2 > 0){																// ak je vysledok len malicko cez limit, skusim to zobrat ako vypoctovu odchylku
		if((abs(temp2 - 1) ) < 0.001) temp2 = (double)1;
	}else{
		if((abs(temp2 + 1) ) < 0.001) temp2 = (double)(-1);
	}
	
	if ((temp2 > 1) || (temp2 < -1)){											// test	, cos uhla musi byt medzi <-1,1>
		return MyRoboArm_IK_Result(false);
	}
		
	// uhol q1 vyratany
	double temp_angle2_d = radsToDegrees(acos(temp2));
	if(abs(temp_angle2_d) < 0.001) temp_angle2_d = (double)0.0;					// mozem zanedbat take male cislo

	double q1 = temp_angle1_d + temp_angle2_d;

	// este sa musi vyratat q2
	//---
	double temp3 = (bPow2 - (a3*a3) - mPow2)/((-2)*a3*sqrt(mPow2));				// cosinusova veta, temp3 je cos(uhla) oproti bPow2 v trojuholniku sqrt(bPow2)---sqrt(mPow2)---a3,  a spolu s temp_angle4_d tvoria sucet, ktory po odratani od 180 dostanem q2

	if(temp3 > 0){																// ak je vysledok len malicko cez limit, skusim to zobrat ako vypoctovu odchylku
		if((abs(temp3 - 1) ) < 0.001) temp3 = (double)1;
	}else{
		if((abs(temp3 + 1) ) < 0.001) temp3 = (double)(-1);
	}

	if ((temp3 > 1) || (temp3 < -1)){											// test	, cos uhla musi byt medzi <-1,1>
		return MyRoboArm_IK_Result(false);
	}

	double temp_angle3_d = radsToDegrees(acos(temp3));
	
	//---
	double temp4 = ((a5*a5) - (a4*a4) - mPow2)/((-2)*a4*sqrt(mPow2));			// cosinusova veta, temp2 je cos(uhla) oproti a5 v trojuholniku sqrt(mPow2)---a4---a5, a spolu s temp_angle3_d tvoria sucet, ktory po odratani od 180 dostanem q2
		
	if(temp4 > 0){																// ak je vysledok len malicko cez limit, skusim to zobrat ako vypoctovu odchylku
		if((abs(temp4 - 1) ) < 0.001) temp4 = (double)1;
	}else{
		if((abs(temp4 + 1) ) < 0.001) temp4 = (double)(-1);
	}

	if ((temp4 > 1) || (temp4 < -1)){											// test	, cos uhla musi byt medzi <-1,1>
		return MyRoboArm_IK_Result(false);
	}
	
	// uhol q2 vyratany
	double temp_angle4_d = radsToDegrees(acos(temp4));
	double q2 = 180 - (temp_angle3_d + temp_angle4_d);

	// uz sa len vrati vysledok v spravnej forme
	double q4 = servos[4]->getAngle_degrees();									// treba len kvoli doplneniu v IK vysledku
	double q5 = servos[5]->getAngle_degrees();									// treba len kvoli doplneniu v IK vysledku
	
	return MyRoboArm_IK_Result(true, q0, q1, q2, q3, q4, q5);
}

bool MyRoboticArm::trySetOneServo(int id, double q){
	/*
	 *  moze sa stat ze je cislo napr.: -0.00...1 a nezoberie ho ako >= 0 apod. ,
	 *  pricom je to este stale dobre-prijatelne riesenie,
	 *  preto sa to pokusi s len jemne upravenym uhlom
	 */

	if(!(servos[id]->set_servo_by_degrees(q))){
		if(!(servos[id]->set_servo_by_degrees(trunc(q,10)))){
			if(!(servos[id]->set_servo_by_degrees(trunc(q,5)))){
				if(!(servos[id]->set_servo_by_degrees(floor(q + 0.5)))){
					return false;
				}
			}
		}
	}
	return true;
}

// IK functionality needs prefered value for q3 (aka angle for servo[3]), this functions tries only with this angle
bool MyRoboticArm::find_solution_and_do_InverseKinematics(Point3d point, double q3){
 	// compute the angles for servo (DOFs) by IK
	MyRoboArm_IK_Result result = InverseKinematics(point, q3);

	// test if the goal is reachable
	if(!(result.reachable)){
		return false;
	}

	// zalohovat povodne nastavenia, ked sa niektore nove nebude dat nastavit, tak sa vratia tieto povodne
	int s0_signal_value = servos[0]->get_akt_signal_value();
	int s1_signal_value = servos[1]->get_akt_signal_value();
	int s2_signal_value = servos[2]->get_akt_signal_value();
	int s3_signal_value = servos[3]->get_akt_signal_value();
	//int s4_signal_value = servos[4]->get_akt_signal_value();		// to je to rotacne servo, nema zatial na neho vplyv inverzna kinematika
	//int s5_signal_value = servos[5]->get_akt_signal_value();		// gripper - effector

	// pokusi sa nastavit postupne vsetky serva
	bool allSetOk = true;
	allSetOk &= trySetOneServo(0, result.q0);
	allSetOk &= trySetOneServo(1, result.q1);
	allSetOk &= trySetOneServo(2, result.q2);
	allSetOk &= trySetOneServo(3, result.q3);
	// trySetOneServo(4, result.q0);	// to je to rotacne servo, nema zatial na neho vplyv inverzna kinematika
	// trySetOneServo(0, result.q0);	// gripper - effector

	// ak sa nieco nepodarilo nastavit tak najskor vrati povodne nastavenia signalov pre serva
	if(!allSetOk){
		servos[0]->set_servo_by_signal_value(s0_signal_value);
		servos[1]->set_servo_by_signal_value(s1_signal_value);
		servos[2]->set_servo_by_signal_value(s2_signal_value);
		servos[3]->set_servo_by_signal_value(s3_signal_value);
		//servos[4]->set_servo_by_signal_value(s4_signal_value);
		//servos[5]->set_servo_by_signal_value(s5_signal_value);
		return false;
	}

	return true;
}

// IK functionality needs prefered value for q3 (aka angle for servo[3]), this functions tries first time with this angle, then tries with the whole scale of servo[3] angle range
bool MyRoboticArm::find_IK_solution_with_prefered_angle_and_do_IK(Point3d point, double q3){
	if(!servos[3]->isInRange_degrees(q3)){
		q3 = servos[3]->getMedianDegree();
		// iny pristup je ze sa v takomto pripade vrati false ... uvidim
	}

	// najskor sa skusi najst riesenie IK pre preferovany uhol
	if(find_solution_and_do_InverseKinematics(point, q3)){
		// najde, ok, v takom pripade je vsetko uz nastavene a staci return true
		return true;
	}

	// ak nenajde, pokusi sa hladat pre dany bod riesenia IK pre upravene hodnoty preferovaneho uhla q3
	double q3_down = q3, q3_up = q3;
	bool test_if_DOWN_in_range = true, test_if_UP_in_range = true;

	// hodnota zmeny zacne od cisla ... a pri kazdom cykle sa zmensi o 1/2
	int change_value = 10;

	// hladanie mozneho riesenia v okoli q3
	while(change_value > 0){
		// zbehne pre cely rozsah, od hodnoty q3 a hore a dole o hodnotu change_value
		while(test_if_DOWN_in_range||test_if_UP_in_range){		
			// ak bola predosla hodnota este v rozsahu serva tak sa zmensi hodnota pre DOWN
			if(test_if_DOWN_in_range){
				q3_down -= (double)change_value;
				test_if_DOWN_in_range = servos[3]->isInRange_degrees(q3_down);
			}

			// ak bola predosla hodnota este v rozsahu serva tak sa zvacsi hodnota pre UP
			if(test_if_UP_in_range){
				q3_up += (double)change_value;
				test_if_UP_in_range = servos[3]->isInRange_degrees(q3_up);
			}
		
			// skusi sa upraveny uhol q3 zmensujuci sa
			if(test_if_DOWN_in_range){
				if(find_solution_and_do_InverseKinematics(point, q3_down)){
					return true;
				}
			}
		
			// skusi sa upraveny uhol q3 zvacsujuci sa
			if(test_if_UP_in_range){
				if(find_solution_and_do_InverseKinematics(point, q3_up)){
					return true;
				}
			}
		}

		// novy cyklus, zjemnenie change_value az kym > 0
		change_value /= 2;

		// znovu nastavenie
		q3_down = q3;
		q3_up = q3;
		test_if_DOWN_in_range = true;
		test_if_UP_in_range = true;
	}

	return false;
}

string MyRoboticArm::getOneServosPacketCommand(int index, int time = 2000){
	// protocol info: http://www.lynxmotion.com/images/html/build136.htm
	
	// <time> Time in mS for the entire move, affects all channels, 65535 max (Optional)
	if((time < 0)||(time > 65535)){
		time = 2000;
	}

	// result string
	stringstream res;

	if((index < 0)||(index > 5)){
		// all SSC-32 commands must end with a carriage return character (ASCII 13).
		char cr_char = 13;
		res << cr_char;

		return res.str();
	}
	
	res << '#' << index << " P" << servos[index]->get_akt_signal_value() << " T" << time << " ";
	
	// all SSC-32 commands must end with a carriage return character (ASCII 13).
	char cr_char = 13;

	res << cr_char;

	return res.str();
}

string MyRoboticArm::getAllServosPacketCommand(int time = 2000, bool init = false){
	// protocol info: http://www.lynxmotion.com/images/html/build136.htm

	// <time> Time in mS for the entire move, affects all channels, 65535 max (Optional)
	if((time < 0)||(time > 65535)){
		time = 2000;
	}

	// result string
	stringstream res;

	for(int i = 0; i < 6; i++){
		res << '#' << i << " P" << servos[i]->get_akt_signal_value() << " ";	// v konstruktore triedy Servo sa priradi do akt.hodnoty jeho min. hodnota
	}

	/*	Important! The first positioning command should be a normal "# <ch> P <pw>" command.
		Because the controller doesn't know where the servo is positioned on powerup, it will
		ignore speed and time commands until the first normal command has been received.		*/


	if(!init){
		res << "T" << time << " ";
	}
	
	// all SSC-32 commands must end with a carriage return character (ASCII 13).
	char cr_char = 13;

	res << cr_char;

	return res.str();
}

#endif