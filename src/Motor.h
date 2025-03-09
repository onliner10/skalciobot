#pragma once
#include <Arduino.h>
#include <PID_v1.h>
#include "config.h"

class Motor {
private:
    const int in1Pin;
    const int in2Pin;
    const int encoderPin;
    
    volatile uint32_t pulseCount = 0;
    bool direction = true; // true = forward
    
    // PID variables
    double pidInput = 0;
    double pidOutput = 0;
    double pidSetpoint = 0;
    PID pidController;
    
    // RPM calculation
    unsigned long lastRpmCalc = 0;
    double currentRpm = 0;
    uint32_t lastPulseCount = 0;
    
    static const int PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)

    static void IRAM_ATTR encoderISR(void* arg);
    void calculateRpm();
    void applyOutput();

public:
    Motor(int pin1, int pin2, int encPin);
    
    void begin();
    void setRpm(double rpm);   // Desired RPM (-MAX_RPM to MAX_RPM)
    void update();            // Call in loop
    void stop();
    
    double getRpm() const { return currentRpm; }
    uint32_t getPulseCount() const { return pulseCount; }
    void resetPulseCount() { pulseCount = 0; }

    // Raw PWM control (-255 to 255)
    void setPwm(int16_t pwm) {
        pwm = constrain(pwm, -255, 255);
        if (pwm > 0) {
            analogWrite(in1Pin, pwm);
            analogWrite(in2Pin, 0);
        } else {
            analogWrite(in1Pin, 0);
            analogWrite(in2Pin, -pwm);
        }
    }
};
