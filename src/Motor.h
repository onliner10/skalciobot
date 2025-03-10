#pragma once
#include <Arduino.h>
#include "config.h"
#include "Logger.h"

class Motor {
private:
    const int in1Pin;
    const int in2Pin;
    const int encoderPin;
    Logger& logger;
    
    // Basic control members
    volatile uint32_t pulseCount = 0;
    volatile unsigned long lastPulseTime = 0;
    int16_t currentPwm = 0;
    
    static const int PWM_RESOLUTION = 8;
    static void IRAM_ATTR encoderISR(void* arg);

    // PID control
    const float KP;
    const float KI;
    const float KD;
    
    float targetRPM = 0;
    float currentRPM = 0;
    float lastError = 0;
    float integral = 0;
    unsigned long lastUpdateTime = 0;
    unsigned long lastPwmUpdateTime = 0;
    static constexpr unsigned long PWM_UPDATE_INTERVAL = 50;
    
    static constexpr float PULSES_PER_REV = MOTOR_PULSES_PER_REV;
    static constexpr float MAX_RPM = MOTOR_MAX_RPM;

    void updatePID();
    void setRawPwm(int pwm);

public:
    Motor(int pin1, int pin2, int encPin, Logger& log, float kp, float ki, float kd);
    void begin();
    void update();
    void stop();
    
    void setRPM(float rpm);
    void setPwm(int pwm); // Keep for testing/recovery
    
    // Getters
    float getCurrentRPM() const { return currentRPM; }
    float getTargetRPM() const { return targetRPM; }
    uint32_t getPulseCount() const { return pulseCount; }
    unsigned long getTimeSinceLastPulse() const { return millis() - lastPulseTime; }
    int16_t getCurrentPwm() const { return currentPwm; }
};
