#include "Motor.h"

// Update constructor
Motor::Motor(int pin1, int pin2, int encPin, Logger& log)
    : in1Pin(pin1), in2Pin(pin2), encoderPin(encPin), logger(log) {
}

void IRAM_ATTR Motor::encoderISR(void* arg) {
    Motor* motor = static_cast<Motor*>(arg);
    motor->accumulatedPulses++;
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

float Motor::getCurrentSpeed() {
    unsigned long now = millis();
    unsigned long timeSinceLastUpdate = now - lastSpeedUpdate;
    
    if (timeSinceLastUpdate >= MOTOR_UPDATE_INTERVAL) {
        // Calculate instantaneous speed
        float instantSpeed = (float)accumulatedPulses * (MOTOR_UPDATE_INTERVAL / (float)timeSinceLastUpdate);
        
        // Update circular buffer
        speedBuffer[speedBufferIndex] = instantSpeed;
        speedBufferIndex = (speedBufferIndex + 1) % SPEED_BUFFER_SIZE;
        
        // Calculate moving average
        float sum = 0;
        for (size_t i = 0; i < SPEED_BUFFER_SIZE; i++) {
            sum += speedBuffer[i];
        }
        currentSpeed = sum / SPEED_BUFFER_SIZE;
        
        // Reset pulse counter
        accumulatedPulses = 0;
        lastSpeedUpdate = now;
    }
    
    return currentSpeed;
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
}

void Motor::stop() {
    setPwm(0);
}