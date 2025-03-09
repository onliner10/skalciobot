#pragma once
#include <Arduino.h>

enum SteeringDirection {
    STRAIGHT = 0,
    LEFT = 1,
    RIGHT = 2
};

class MotorController {
private:
    const int steeringMotor1, steeringMotor2;
    const int driveMotor1, driveMotor2;
    const int motorFault, motorSleep, motorFlt;
    int currentSpeed = 0;
    SteeringDirection currentSteering = STRAIGHT;
    static const int PWM_FREQUENCY = 500;  // 500Hz PWM frequency
    static const int PWM_DEADBAND = 5;     // Small deadband to prevent noise
    static const int PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)
    bool isFaulted = false;
    static const int MAX_FAULT_RECOVERY_ATTEMPTS = 3;
    static const unsigned long FAULT_RESET_DELAY = 100; // ms
    unsigned int faultCount = 0;
    unsigned long lastFaultTime = 0;
    
    bool attemptFaultRecovery();

public:
    MotorController(
        int steeringM1, int steeringM2,
        int driveM1, int driveM2,
        int mFault, int mSleep, int mFlt
    );

    void begin();
    void setSpeed(int speed);      // -100 to 100
    void setSteering(SteeringDirection direction);
    void stop();
    bool checkFault();
    void sleep(bool enable);
    bool isRecoverable() const { return faultCount < MAX_FAULT_RECOVERY_ATTEMPTS; }
    void resetFaultCount();  // Remove inline implementation
};