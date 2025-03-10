#pragma once
#include "Motor.h"

class MotorController {
private:
    Motor& leftMotor;
    Motor& rightMotor;
    const int sleepPin;
    const int faultPin;
    
    int currentPwm = 0;        // Current PWM value (-255 to 255)
    float currentSteering = 0; // -1 (full left) to 1 (full right)
    bool isFaulted = false;
    
    void updateMotors();

public:
    MotorController(Motor& left, Motor& right, int slp, int flt);
    
    void begin();
    void setPwm(int pwm);            // -255 to 255
    void setSteering(float steering); // -1.0 to 1.0
    void update();                    // Call in loop
    void stop();
    bool checkFault();
    void sleep(bool enable);
    
    int getPwm() const { return currentPwm; }
    float getSteering() const { return currentSteering; }

    // For stuck detection
    unsigned long getLeftTimeSinceLastPulse() const { return leftMotor.getTimeSinceLastPulse(); }
    unsigned long getRightTimeSinceLastPulse() const { return rightMotor.getTimeSinceLastPulse(); }

    void enable() { digitalWrite(sleepPin, HIGH); }
    void disable() { digitalWrite(sleepPin, LOW); }
    bool isFault() const { return digitalRead(faultPin) == LOW; }

    void test() {
        digitalWrite(sleepPin, HIGH);  // Enable drivers
        delay(1);
        
        // Test forward
        leftMotor.setPwm(128);
        rightMotor.setPwm(128);
        delay(1000);
        
        // Test backward
        leftMotor.setPwm(-128);
        rightMotor.setPwm(-128);
        delay(1000);
        
        stop();
    }
};