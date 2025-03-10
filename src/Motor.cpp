#include "Motor.h"

Motor::Motor(int pin1, int pin2, int encPin, Logger& log)
    : in1Pin(pin1), in2Pin(pin2), encoderPin(encPin), logger(log) {
}

void IRAM_ATTR Motor::encoderISR(void* arg) {
    Motor* motor = static_cast<Motor*>(arg);
    motor->pulseCount++;
    motor->lastPulseTime = millis();
}

void Motor::begin() {
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(encoderPin, INPUT_PULLUP);
    
    // Initialize outputs to zero
    analogWrite(in1Pin, 0);
    analogWrite(in2Pin, 0);
    
    analogWriteResolution(PWM_RESOLUTION);
    
    // Setup encoder interrupt
    attachInterruptArg(
        digitalPinToInterrupt(encoderPin),
        encoderISR,
        this,
        CHANGE
    );
}

void Motor::setPwm(int pwm) {
    pwm = constrain(pwm, -255, 255);
    currentPwm = pwm;
    
    if (pwm >= 0) {
        analogWrite(in2Pin, 0);
        analogWrite(in1Pin, pwm);
    } else {
        analogWrite(in1Pin, 0);
        analogWrite(in2Pin, -pwm);
    }
    
    logger.debug("Motor PWM: " + String(pwm), LogContext::Motor);
}

void Motor::stop() {
    currentPwm = 0;
    analogWrite(in1Pin, 0);
    analogWrite(in2Pin, 0);
}