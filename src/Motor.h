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
    
    volatile uint32_t accumulatedPulses = 0;  // Total pulses since last read
    volatile unsigned long lastPulseTime = 0;
    unsigned long lastSpeedUpdate = 0;
    float currentSpeed = 0.0f;         // Store last calculated speed
    int16_t currentPwm = 0;
    
    static const int PWM_RESOLUTION = 8;
    static void IRAM_ATTR encoderISR(void* arg);
    
    static constexpr float PULSES_PER_REV = MOTOR_PULSES_PER_REV;
    static constexpr size_t SPEED_BUFFER_SIZE = 4;  // Number of samples to average
    float speedBuffer[SPEED_BUFFER_SIZE] = {0};
    size_t speedBufferIndex = 0;
    
public:
    Motor(int pin1, int pin2, int encPin, Logger& log);
    void begin();
    void setPwm(int pwm);  // -255 to 255
    void stop();
    
    float getCurrentSpeed();  // Returns pulses per interval
    uint32_t getPulseCount() const { return accumulatedPulses; }  // For diagnostics only
    unsigned long getTimeSinceLastPulse() const { return millis() - lastPulseTime; }
    int16_t getCurrentPwm() const { return currentPwm; }
};
