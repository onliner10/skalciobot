#include "Motor.h"

// Update constructor
Motor::Motor(int pin1, int pin2, int encPin, Logger& log, float kp, float ki, float kd)
    : in1Pin(pin1), in2Pin(pin2), encoderPin(encPin), logger(log),
      KP(kp), KI(ki), KD(kd) {
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
        RISING
    );
}

void Motor::update() {
    unsigned long now = millis();
    
    // Update RPM measurement every 50ms
    if (now - lastUpdateTime >= 50) {
        float deltaTime = (now - lastUpdateTime) / 1000.0f;
        float revolutions = pulseCount / PULSES_PER_REV;
        currentRPM = (revolutions * 60.0f) / deltaTime;
        
        pulseCount = 0;
        lastUpdateTime = now;
    }
    
    if (now - lastPwmUpdateTime >= PWM_UPDATE_INTERVAL) {
        if (targetRPM != 0) {
            updatePID();
        }
        lastPwmUpdateTime = now;
    }
}

void Motor::updatePID() {
    float error = targetRPM - currentRPM;
    float deltaTime = PWM_UPDATE_INTERVAL / 1000.0f;

    // Reset integral when target changes direction
    if (signbit(targetRPM) != signbit(lastError)) {
        integral = 0;
    }
    
    integral += error * deltaTime;
    integral = constrain(integral, -100.0f, 100.0f);  // Limit integral windup
    
    // Simple PID calculation
    float derivative = (error - lastError) / deltaTime;
    float output = (KP * error) + (KI * integral) + (KD * derivative);
    
    // Ensure output direction matches target
    if (targetRPM >= 0) {
        output = constrain(output, 0, 255);
    } else if (targetRPM < 0) {
        output = constrain(output, -255, 0);
    }
    
    setRawPwm(int(output));
    lastError = error;
}

void Motor::setRPM(float rpm) {
    targetRPM = constrain(rpm, -MAX_RPM, MAX_RPM);
}

void Motor::setRawPwm(int pwm) {
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

// Keep existing setPwm method for testing/recovery
void Motor::setPwm(int pwm) {
    targetRPM = 0; // Disable PID control
    setRawPwm(pwm);
}

void Motor::stop() {
    targetRPM = 0;
    integral = 0;
    lastError = 0;
    setRawPwm(0);
}