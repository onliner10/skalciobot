#pragma once
#include "Motor.h"
#include "RobotState.h"
#include "Logger.h"
#include "config.h"
#include "TimerManager.h"

class MotorController : public TimerCallback {
private:
    Motor& leftMotor;
    Motor& rightMotor;
    const int faultPin;
    Logger& logger;
    RobotState& state;
    
    // Base speed control
    float speedPercent = 0;
    
    // Steering control
    float targetSteeringRatio = 0;
    float currentSteering = 0;
    
    // PID control for steering
    const float KP = STEERING_PID_KP;
    const float KI = STEERING_PID_KI;
    const float KD = STEERING_PID_KD;
    float steeringError = 0;
    float steeringIntegral = 0;
    float lastSteeringError = 0;

    void updatePID();
    void applyMotorOutputs(float leftPwm, float rightPwm);
    float calculateCurrentSteeringRatio() const;

    float leftMotorScale = DEFAULT_LEFT_MOTOR_SCALE;
    float rightMotorScale = DEFAULT_RIGHT_MOTOR_SCALE;
    void calibrateMotors();

public:
    MotorController(Motor& left, Motor& right, int flt, RobotState& s, Logger& log);
    ~MotorController();
    void begin();
    void setSteering(float steering);
    void stop();
    bool checkFault();
    
    float getSteering() const { return currentSteering; }
    unsigned long getLeftTimeSinceLastPulse() const { return leftMotor.getTimeSinceLastPulse(); }
    unsigned long getRightTimeSinceLastPulse() const { return rightMotor.getTimeSinceLastPulse(); }
    bool isFault() const { return digitalRead(faultPin) == LOW; }
    void setSpeedPercent(float percent);
    float getSpeedPercent() const { return speedPercent; }
    void test();
    void calibrate() { calibrateMotors(); }
    float getLeftScale() const { return leftMotorScale; }
    float getRightScale() const { return rightMotorScale; }
    void onTimer() override { updatePID(); }
};