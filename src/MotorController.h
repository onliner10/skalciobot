#pragma once
#include "Motor.h"
#include "RobotState.h"
#include "Logger.h"

class MotorController {
private:
    Motor& leftMotor;
    Motor& rightMotor;
    const int sleepPin;
    const int faultPin;
    Logger& logger;
    
    int currentPwm = 0;        // Current PWM value (-255 to 255)
    float currentSteering = 0; // -1 (full left) to 1 (full right)
    bool isFaulted = false;
    float currentRPM = 0;      // Base RPM value
    static constexpr float MAX_RPM = 600.0f;
    
    void updateMotors();
    RobotState& state;

public:
    MotorController(Motor& left, Motor& right, int slp, int flt, RobotState& s, Logger& log);
    
    void begin();
    void setPwm(int pwm);            // -255 to 255
    void setSteering(float steering); // -1.0 to 1.0
    void update();                    // Call in loop
    void stop();
    bool checkFault();
    void sleep(bool enable);
    
    int getPwm() const { return currentPwm; }
    float getSteering() const { return currentSteering; }
    void setRPM(float rpm);            // -MAX_RPM to MAX_RPM
    float getRPM() const { return currentRPM; }

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