#pragma once
#include "Motor.h"
#include "RobotState.h"
#include "Logger.h"
#include "config.h"

class MotorController {
private:
    Motor& leftMotor;
    Motor& rightMotor;
    const int sleepPin;
    const int faultPin;
    Logger& logger;
    RobotState& state;
    
    // Base speed control
    float speedPercent = 0;  // -100 to 100
    
    // Steering control
    float targetSteeringRatio = 0;  // -1 (full left) to 1 (full right)
    float currentSteering = 0;      // Current steering value
    
    // PID control for steering
    const float KP = STEERING_PID_KP;
    const float KI = STEERING_PID_KI;
    const float KD = STEERING_PID_KD;
    float steeringError = 0;
    float steeringIntegral = 0;
    float lastSteeringError = 0;
    unsigned long lastPidUpdate = 0;
    static constexpr unsigned long PID_UPDATE_INTERVAL = STEERING_PID_INTERVAL;
    
    unsigned long lastPwmUpdate = 0;
    bool shouldUpdatePwm(float leftPwm, float rightPwm) const;
    
    // Cache last PWM values for throttling
    float lastLeftPwm = 0;
    float lastRightPwm = 0;
    float lastOutputLeftPwm = 0;
    float lastOutputRightPwm = 0;
    static constexpr float PWM_SMOOTHING = MOTOR_PWM_SMOOTHING;

    void updatePID();
    void applyMotorOutputs(float leftPwm, float rightPwm);
    float calculateCurrentSteeringRatio() const;

    static constexpr float MAX_RPM = MOTOR_MAX_RPM;

    float leftMotorScale = DEFAULT_LEFT_MOTOR_SCALE;
    float rightMotorScale = DEFAULT_RIGHT_MOTOR_SCALE;
    void calibrateMotors();

public:
    MotorController(Motor& left, Motor& right, int slp, int flt, RobotState& s, Logger& log);
    
    void begin();
    void setPwm(int pwm);            // -255 to 255
    void setSteering(float steering); // -1.0 to 1.0
    void update();                    // Call in loop
    void stop();
    bool checkFault();
    void sleep(bool enable);
    
    float getSteering() const { return currentSteering; }
    
    // Remove setRPM and getRPM methods
    
    // For stuck detection
    unsigned long getLeftTimeSinceLastPulse() const { return leftMotor.getTimeSinceLastPulse(); }
    unsigned long getRightTimeSinceLastPulse() const { return rightMotor.getTimeSinceLastPulse(); }

    void enable() { digitalWrite(sleepPin, HIGH); }
    void disable() { digitalWrite(sleepPin, LOW); }
    bool isFault() const { return digitalRead(faultPin) == LOW; }

    void setSpeedPercent(float percent);  // -100 to 100
    float getSpeedPercent() const { return speedPercent; }

    void test();  // Add test method declaration

    void calibrate() { calibrateMotors(); }
    void setMotorScales(float left, float right) {
        leftMotorScale = left;
        rightMotorScale = right;
    }

    float getLeftScale() const { return leftMotorScale; }
    float getRightScale() const { return rightMotorScale; }
};