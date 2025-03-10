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
    
    volatile uint32_t pulseCount = 0;
    volatile unsigned long lastPulseTime = 0;
    int16_t currentPwm = 0;
    
    static const int PWM_RESOLUTION = 8;
    static void IRAM_ATTR encoderISR(void* arg);

public:
    Motor(int pin1, int pin2, int encPin, Logger& log);
    void begin();
    void setPwm(int pwm);
    void update() {}  // Empty, no RPM calculation needed
    void stop();
    
    // Getters for diagnostics only
    uint32_t getPulseCount() const { return pulseCount; }
    unsigned long getTimeSinceLastPulse() const { return millis() - lastPulseTime; }
    int16_t getCurrentPwm() const { return currentPwm; }
};
