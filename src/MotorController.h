#pragma once
#include "Motor.h"

class MotorController {
private:
    Motor& leftMotor;
    Motor& rightMotor;
    const int sleepPin;
    const int faultPin;
    
    double currentRpm = 0;      // Overall RPM
    float currentSteering = 0;  // -1 (full left) to 1 (full right)
    bool isFaulted = false;
    
    void updateMotors();

public:
    MotorController(Motor& left, Motor& right, int slp, int flt);
    
    void begin();
    void setRpm(double rpm);          // -MAX_RPM to MAX_RPM
    void setSteering(float steering); // -1.0 to 1.0
    void update();                    // Call in loop
    void stop();
    bool checkFault();
    void sleep(bool enable);
    
    double getRpm() const { return currentRpm; }
    float getSteering() const { return currentSteering; }

    // Add getters for motor RPMs
    double getLeftRpm() const { return leftMotor.getRpm(); }
    double getRightRpm() const { return rightMotor.getRpm(); }

    void test() {
        setRpm(100);  // Run forward
        delay(1000);
        setRpm(-100); // Run backward
        delay(1000);
        stop();       // Stop motors
    }
};