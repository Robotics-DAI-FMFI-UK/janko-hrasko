#ifndef ROBOT_H
#define	ROBOT_H

using namespace std;

//---------------------------------------------------------------------------

double radsToDegrees(double rads) {
    return (180 * rads) / M_PI;
}

double degreesToRads(double degrees) {
    return (degrees * M_PI) / (double) 180;
}

double trunc(double value, int decimal_places) {
    double integer = 0, fractional = 0, output = 0;
    int j = 0, places = 1;

    fractional = modf(value, &output);
    for (int i = 0; i < decimal_places + 1; i++) {
        fractional = modf(fractional, &integer);
        for (j = 0; j < i; j++) {
            places *= 10;
        }
        output += integer / places;
        places = 1;
        fractional *= 10;
    }
    return output;
}

/* -----------  STRUCT MYPOINT  --------------- */

struct MyPoint {
    double x, y, z;

    MyPoint(double nx = 0, double ny = 0, double nz = 0) : x(nx), y(ny), z(nz) {
    };

    bool operator!=(MyPoint p) {
        return ((x != p.x) || (y != p.y) || (z != p.z));
    };
};

/* -----------  end struct MyPoint  ---------- */

/* -----------  STRUCT ROBOARMIKRESULT  ---- */

struct RoboArm_IK_Result {
    double q1, q2, q3, q4, q5;
    bool reachable;

    RoboArm_IK_Result(bool nReachable = true, double nq1 = 0, double nq2 = 0, double nq3 = 0, double nq4 = 0, double nq5 = 0) : reachable(nReachable), q1(nq1), q2(nq2), q3(nq3), q4(nq4), q5(nq5) {
    };
};

/* -----------  end struct RoboArm_IK_Result  ---------- */

/* -----------  CLASS MOTOR  ----------------- */

class Motor {
private:
    int direction; // MOTOR_FWD 1 , MOTOR_BWD 2 , MOTOR_BRAKE 3 , MOTOR_FLOAT 4
    int speed; // 0 - 255
public:

    Motor() : direction(4), speed(0) {
    };
    //~Motor(){};

    bool set_motor_direction(int nDir) {
        if ((nDir < 1) || (nDir > 4)) return false;
        direction = nDir;
        return true;
    };

    bool set_motor_speed(int nSpeed) {
        if ((nSpeed < 0) || (nSpeed > 255)) return false;
        speed = nSpeed;
        return true;
    };

    bool set_motor_dir_speed(int nD, int nS) {
        return (set_motor_direction(nD) && set_motor_speed(nS));
    };

    const int getDirection() {
        return direction;
    };

    const int getSpeed() {
        return speed;
    };
};

/* -----------  end class MOTOR  ---------- */

/* -----------  CLASS SERVO  ----------------- */

class Servo {
public:
    double min_degrees; // (konkretne v tomto pripade pre vsetky servo motory ramena = 0 stupnov)
    double max_degrees;
    int signal_value_for_min_deg; // hodnota servo motora pre uhol = min rozpetie stupnov
    int signal_value_for_max_deg; // hodnota servo motora pre uhol = max rozpetie stupnov
    double rozpatie; // rozpatie_servo_motora v uhloch
    int akt_signal_value; // aktualna_hodnota_servo_motora
public:
    Servo();
    Servo(double mid, int svmid, double mxd, int svmxd);

    const double conversion_signal_value_to_degrees(int value);
    const int conversion_degrees_to_signal_value(int degrees);

    const bool isInRange_signal_value(int value);
    const bool isInRange_degrees(double degrees);

    bool set_servo_by_signal_value(int nValue);
    bool set_servo_by_degrees(double nDegrees);

    const int get_akt_signal_value() {
        return akt_signal_value;
    };
    const double getAngle_degrees();
    const double getAngle_radians();
};

Servo::Servo() {
}

Servo::Servo(double mid, int svmid, double mxd, int svmxd) : min_degrees(mid), signal_value_for_min_deg(svmid), max_degrees(mxd), signal_value_for_max_deg(svmxd) {
    rozpatie = max_degrees - min_degrees;

    akt_signal_value = signal_value_for_min_deg;
}

// spravny vysledok len ak hodnoty v prislusnom rozpati

const double Servo::conversion_signal_value_to_degrees(int value) {
    return min_degrees + (abs(signal_value_for_min_deg - value) * rozpatie) / (double) (abs(signal_value_for_min_deg - signal_value_for_max_deg));
}

const int Servo::conversion_degrees_to_signal_value(int degrees) {
    double temp = ((degrees - min_degrees) * abs(signal_value_for_min_deg - signal_value_for_max_deg)) / (double) rozpatie;
    return (signal_value_for_min_deg < signal_value_for_max_deg) ? floor((temp + signal_value_for_min_deg) + 0.5) : floor((signal_value_for_min_deg - temp) + 0.5);
}

const bool Servo::isInRange_signal_value(int value) {
    if ((value <= signal_value_for_min_deg)&&(value <= signal_value_for_max_deg)) {
        return false;
    }
    if ((value >= signal_value_for_min_deg)&&(value >= signal_value_for_max_deg)) {
        return false;
    }
    return true;
}

const bool Servo::isInRange_degrees(double degrees) {
    cout << "degrees: " << degrees;
    if ((degrees <= min_degrees) || (degrees >= max_degrees)) {
        cout << "false" << endl;
        return false;
    }
    cout << "true" << endl;
    return true;
}

bool Servo::set_servo_by_signal_value(int nValue) {
    if (isInRange_signal_value(nValue)) {
        akt_signal_value = nValue;
        return true;
    }
    return false;
}

bool Servo::set_servo_by_degrees(double nDegrees) {
    if (isInRange_degrees(nDegrees)) {
        akt_signal_value = conversion_degrees_to_signal_value(nDegrees);
        return true;
    }
    return false;
}

const double Servo::getAngle_degrees() {
    //(abs(zero_deg_value - akt_value) * rozpatie) / (double)(abs(zero_deg_value - max_deg_value));
    return conversion_signal_value_to_degrees(akt_signal_value);
}

const double Servo::getAngle_radians() {
    return degreesToRads(getAngle_degrees());
}

/* -----------  end class Servo  ------------- */

/* -----------  CLASS ROBOTIC_ARM  -------- */

class RoboticArm {
private:
    Servo servos[5]; // servos[0] je servo 1 (GRIPPER), servos[1] je servo 2, servos[2] je servo 3, servos[3] je servo 4, servos[4] je servo 5
    int links[5]; // mierka v milimetroch: links[0] je dlzka linku od joint 1 k 2, links[1] je dlzka linku od joint 2 k 3, ...
public:
    RoboticArm();

    /*~RoboticArm() {
        delete [] servos;
    };*/

    int* getServoActualPositions() {
        int* servoActualPositions = new int[5];
        servoActualPositions[0] = servos[0].get_akt_signal_value();
        servoActualPositions[1] = servos[1].get_akt_signal_value();
        servoActualPositions[2] = servos[2].get_akt_signal_value();
        servoActualPositions[3] = servos[3].get_akt_signal_value();
        servoActualPositions[4] = servos[4].get_akt_signal_value();

        return servoActualPositions;
    }

    bool setServo_i_degrees(int i, double degrees);

    const MyPoint ForwardKinematics(double x = 0, double y = 0, double z = 0) {
        return ForwardKinematics(MyPoint(x, y, z));
    }
    const MyPoint ForwardKinematics(MyPoint point);

    const RoboArm_IK_Result InverseKinematics(double x, double y, double z, double q2) {
        return InverseKinematics(MyPoint(x, y, z), q2);
    };
    const RoboArm_IK_Result InverseKinematics(MyPoint point, double q2);

    bool find_solution_and_do_InverseKinematics(double x, double y, double z, double q2) {
        return find_solution_and_do_InverseKinematics(MyPoint(x, y, z), q2);
    };
    bool find_solution_and_do_InverseKinematics(MyPoint point, double q2);

    string sendServoPacket(char co, int i);
};

RoboticArm::RoboticArm() {
    servos[0] = Servo(0, 90, 30, 210); // Gripper(aj End Effector) funguje rovnako ako rotation serva, len namiesto uhlov pracuje so sirkov chapadla, v tomto pripade 30 mm su najsirsie, 0 su zatvorene
    servos[1] = Servo(0, 232, 125, 110);
    servos[2] = Servo(0, 60, 151, 210);
    servos[3] = Servo(0, 3300, 90, 2190);
    servos[4] = Servo(-180, 3450, 13, 670);

    // mierka v milimetroch: links[0] je dlzka linku od joint 1 k 2, links[1] je dlzka linku od joint 2 k 3, ...
    links[0] = 75;
    links[1] = 125;
    links[2] = 118;
    links[3] = 22;
    links[4] = 45;
}

bool RoboticArm::setServo_i_degrees(int i, double degrees) {
    if ((i < 0) || (i > 4)) {
        return false;
    }
    return servos[i].set_servo_by_degrees(degrees);
}

const MyPoint RoboticArm::ForwardKinematics(MyPoint point) {
    double q2 = servos[1].getAngle_radians();
    double q3 = servos[2].getAngle_radians();
    double q4 = servos[3].getAngle_radians();
    double q5 = servos[4].getAngle_radians();

    double a1 = links[0];
    double a2 = links[1];
    double a3 = links[2];
    double d4 = links[3];
    double d5 = links[4];

    double x = 0, y = 0, z = 0;
    if (point != MyPoint()) {
        x = (-1) * sin(q5) * sin(q2 + q3 + q4) * point.x + (-1) * sin(q5) * cos(q2 + q3 + q4) * point.y + (-1) * cos(q5) * point.z;
        y = (-1) * cos(q5) * sin(q2 + q3 + q4) * point.x + (-1) * cos(q5) * cos(q2 + q3 + q4) * point.y + (-1) * sin(q5) * point.z;
        z = cos(q2 + q3 + q4) * point.x + (-1) * sin(q2 + q3 + q4) * point.y;
    }

    double temp = a1 * sin(q2 + q3 + q4) + a2 * sin(q3 + q4) + a3 * sin(q4);
    x += (-1) * sin(q5) * temp;
    y += cos(q5) * temp;
    z += a1 * cos(q2 + q3 + q4) + a2 * cos(q3 + q4) + a3 * cos(q4) + d4 + d5;

    return MyPoint(x, y, z);
}

const RoboArm_IK_Result RoboticArm::InverseKinematics(MyPoint point, double q2) {
    double a1 = links[0];
    double a2 = links[1];
    double a3 = links[2];
    double d4 = links[3];
    double d5 = links[4];

    double q1 = servos[0].getAngle_degrees();

    double tZ = point.z - d4 - d5; // bod v transformovanej rovine, je to vyska hldaneho bodu ramena v povodnom surad. systeme na osiZ, ale az od suradn. systemu A4
    double tXY = 0; // bod v transformovanej rovine, je to dlzka ramena v povodnej rovine osX a osY
    double q5 = servos[4].getAngle_degrees();
    if ((point.x < 0) || (point.x > 0) || (point.y < 0) || (point.y > 0)) { // ak su obe rovne nule, atan2 vrati chybu. v takomto pripade je servo[4] lubovolne natocene, preto nech zostane to ako je aktualne a tXY zostane 0
        double temp_atanXY = atan2(point.x, point.y); // normalne sa zadava atan2(y,x), ale teraz --> uhol od osiY k osiX v originalnom suradnicovom systeme, ale *(-1), lebo suradnicovy system serva je otoceny + servo sa pohybuje v protismere
        q5 = (-1) * radsToDegrees(temp_atanXY);
        if (!(servos[4].isInRange_degrees(q5))) {
            return RoboArm_IK_Result(false);
        }

        if (fabs(temp_atanXY - (M_PI / (double) 2)) < 0.000001) { // rovnost realnych cisiel
            // reason: cos 90 == 0
            tXY = point.x / sin(temp_atanXY);
        } else {
            tXY = point.y / cos(temp_atanXY);
        }
    }

    /*
    double temp_angle1 = atan2(y,x);
    double temp_angle1_d = radsToDegrees(temp_angle1);
     */
    double temp_angle1_d = radsToDegrees(atan2(tZ, tXY));

    double mPow2 = (a1 * a1) + (a2 * a2) - 2 * a2 * a1 * cos(degreesToRads(180 - q2));
    double bPow2 = (tXY * tXY) + (tZ * tZ);

    double temp2 = (mPow2 - (a3 * a3) - bPow2) / ((-2) * a3 * sqrt(bPow2));
    if ((temp2 > 1) || (temp2 < -1)) {
        return RoboArm_IK_Result(false);
    } else {
        /*
        double temp_angle2 = acos(temp2);
        double temp_angle2_d = RadsToDegrees(temp_angle2);
         */
        double temp_angle2_d = radsToDegrees(acos(temp2));

        double q4 = 90 - (temp_angle1_d + temp_angle2_d);

        double temp3 = (bPow2 - (a3 * a3) - mPow2) / ((-2) * a3 * sqrt(mPow2));
        if ((temp3 > 1) || (temp3 < -1)) {
            return RoboArm_IK_Result(false);
        } else {
            /*
            double temp_angle3 = acos(temp3);
            double temp_angle3_d = RadsToDegrees(q3);
             */
            double temp_angle3_d = radsToDegrees(acos(temp3));

            double temp4 = ((a1 * a1) - (a2 * a2) - mPow2) / ((-2) * a2 * sqrt(mPow2));
            if ((temp4 > 1) || (temp4 < -1)) {
                return RoboArm_IK_Result(false);
            } else {
                /*
                double temp_angle4 = acos(temp4);
                double temp_angle4_d = RadsToDegrees(q4);
                 */
                double temp_angle4_d = radsToDegrees(acos(temp4));

                double q3 = 180 - (temp_angle3_d + temp_angle4_d);

                return RoboArm_IK_Result(true, q1, q2, q3, q4, q5);
            }
        }
    }
}

bool RoboticArm::find_solution_and_do_InverseKinematics(MyPoint point, double q2) {
    RoboArm_IK_Result result = InverseKinematics(point, q2);
    cout << "IK was: " << result.q2 << " " << result.q3 << " " << result.q4 << " " << result.q5;

    if (!(result.reachable)) {
        return false;
    }

    //servos[1].set_servo_by_degrees(result.q2);
    //servos[2].set_servo_by_degrees(result.q3);
    //servos[3].set_servo_by_degrees(result.q4);
    //servos[4].set_servo_by_degrees(result.q5);

    /*
      moze sa stat ze je cislo napr.: -0.00...1 a nezoberie ho ako >= 0 apod. ,
      pricom je to este stale dobre-prijatelne riesenie,
      preto sa to pokusi s len jemne upravenym uhlom
     */

    /*
    if(!(servos[0].set_servo_by_degrees(result.q1))){
      if(!(servos[0].set_servo_by_degrees(trunc(result.q1,10)))){
        if(!(servos[0].set_servo_by_degrees(trunc(result.q1,5)))){
          if(!(servos[0].set_servo_by_degrees(floor(result.q1 + 0.5)))){
            return false;
          }
        }
      }
    }
     */
    if (!(servos[1].set_servo_by_degrees(result.q2))) {
        if (!(servos[1].set_servo_by_degrees(trunc(result.q2, 10)))) {
            if (!(servos[1].set_servo_by_degrees(trunc(result.q2, 5)))) {
                if (!(servos[1].set_servo_by_degrees(floor(result.q2 + 0.5)))) {
                    cout << "reachable but q2 not set, angle " << result.q2 << " is not in range <" << servos[1].min_degrees << "," << servos[1].max_degrees << ">" << endl;
                    return false;
                }
            }
        }
    }
    if (!(servos[2].set_servo_by_degrees(result.q3))) {
        if (!(servos[2].set_servo_by_degrees(trunc(result.q3, 10)))) {
            if (!(servos[2].set_servo_by_degrees(trunc(result.q3, 5)))) {
                if (!(servos[2].set_servo_by_degrees(floor(result.q3 + 0.5)))) {
                    cout << "reachable but q3 not set, angle " << result.q3 << " is not in range <" << servos[2].min_degrees << "," << servos[2].max_degrees << ">" << endl;
                    return false;
                }
            }
        }
    }
    if (!(servos[3].set_servo_by_degrees(result.q4))) {
        if (!(servos[3].set_servo_by_degrees(trunc(result.q4, 10)))) {
            if (!(servos[3].set_servo_by_degrees(trunc(result.q4, 5)))) {
                if (!(servos[3].set_servo_by_degrees(floor(result.q4 + 0.5)))) {
                    cout << "reachable but q4 not set, angle " << result.q4 << " is not in range <" << servos[3].min_degrees << "," << servos[3].max_degrees << ">" << endl;
                    return false;
                }
            }
        }
    }
    if (!(servos[4].set_servo_by_degrees(result.q5))) {
        if (!(servos[4].set_servo_by_degrees(trunc(result.q5, 10)))) {
            if (!(servos[4].set_servo_by_degrees(trunc(result.q5, 5)))) {
                if (!(servos[4].set_servo_by_degrees(floor(result.q5 + 0.5)))) {
                    cout << "reachable but q5 not set, angle " << result.q5 << " is not in range <" << servos[4].min_degrees << "," << servos[4].max_degrees << ">" << endl;
                    return false;
                }
            }
        }
    }

    return true;
}

string RoboticArm::sendServoPacket(char co, int index) {
    ostringstream result;

    if (co == 's') {
        if ((index < 0) || (index > 4)) {
            return '\0';
        }

        result << ".s";
        result << index + 1; // mikroradice maju aktualne oznacene servomotory od 1 po 5, nie od nuly

        int signal = servos[index].get_akt_signal_value();
        result << signal;

        result << ';';

        return result.str();
    }

    if (co == 'l') {
        result << ".sl;";
        return result.str();
    }

    if (co == 'v') {
        result << ".sv;";
        return result.str();
    }

    if (co == 'b') {
        result << ".sb;";
        return result.str();
    }

    return result.str();
}

/* -----------  end class RoboticArm  ------ */

/* -----------  CLASS ROBOT  -------------- */

class Robot {
private:
    RoboticArm roboticArm = RoboticArm();
    Motor motors[4];
public:
    Robot();
    //~Robot(){delete roboticArm; delete [] motors;};

    int initConnection(string port);

    RoboticArm& getRoboticArm() {
        return roboticArm;
    };

    int* getServoPositions() {
        roboticArm.getServoActualPositions();
    };

    Motor& getMotor(int i) {
        if ((i >= 0) || (i <= 3))
            return motors[i];
    };

    string sendMotorPacket(char co, int index);
};

Robot::Robot() {
    motors[0] = Motor();
    motors[1] = Motor();
    motors[2] = Motor();
    motors[3] = Motor();
}

int Robot::initConnection(string port) {
    struct termios oldtio, newtio;
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open()");
        return 0;
    }
    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof (newtio));
    newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    cout << "port: " << fd << endl;
    return fd;
}

string Robot::sendMotorPacket(char co, int index) {
    ostringstream result;

    if (co == 'm') {
        if ((index < 0) || (index > 3)) {
            return '\0';
        }

        result << ".m";
        result << index + 1; // mikroradice maju aktualne oznacene motory od 1 po 4, nie od nuly

        int dir = motors[index].getDirection();
        result << dir;

        int speed = motors[index].getSpeed();
        result << speed;

        result << ';';

        return result.str();
    }

    if (co == 'l') {
        //flash motor LED
        result << ".ml;";
        return result.str();
    }

    if (co == 'k') {
        //invert motor LED  
        result << ".mk;";
        return result.str();
    }

    if (co == 'v') {
        //print motor states
        result << ".mv;";
        return result.str();
    }

    if (co == 'h') {
        //brake all motors
        result << ".mh;";
        return result.str();
    }

    if (co == 'j') {
        //float all motors
        result << ".mj;";
        return result.str();
    }

    if (co == 'f') {
        //all motors fwd
        result << ".mf;";
        return result.str();
    }

    if (co == 'g') {
        //all motors bwd
        result << ".mg;";
        return result.str();
    }

    return result.str();
}

/* -----------  end class Robot  ----------- */

void sendPacket(int port, string packet) {
    //char toSend[] = packet.c_str();
    bool isServoPacket = (packet[1] == 's');

    //  printf("sending %s...\n", toSend);

    //for (int i = 0; i < strlen(toSend); i++) {
    //for(int i = 0; i < strlen(packet.c_str()); i++){
    //  int written = write(port, &(packet[i]), 1);
    int written = write(port, packet.c_str(), packet.size());
    //cout << "written: " << written << endl;

    if (isServoPacket) {
        usleep(30000); // aby to platilo iba pri servomotoroch, motory ATtiny to zvlada aj bez toho
    }

    if (written < 0) {
        perror("write()");
        return;
    } else if (written == 0)
        cout << "lost char" << endl;
    //}
}

#endif	/* ROBOT_H */

