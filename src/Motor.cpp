#include "Motor.h"

Motor::Motor(int pin1, int pin2, int encPin)
    : in1Pin(pin1), in2Pin(pin2), encoderPin(encPin),
      pidController(&pidInput, &pidOutput, &pidSetpoint,
                   MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD, DIRECT) {
    pidController.SetMode(AUTOMATIC);
    pidController.SetSampleTime(PID_SAMPLE_TIME);
    pidController.SetOutputLimits(-255, 255);
}

void IRAM_ATTR Motor::encoderISR(void* arg) {
    Motor* motor = static_cast<Motor*>(arg);
    motor->pulseCount++;
}

void Motor::begin() {
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(encoderPin, INPUT_PULLUP);
    
    analogWriteResolution(PWM_RESOLUTION);
    
    // Setup encoder interrupt
    attachInterruptArg(
        digitalPinToInterrupt(encoderPin),
        encoderISR,
        this,
        RISING
    );
}

void Motor::calculateRpm() {
    unsigned long now = millis();
    if (now - lastRpmCalc >= RPM_CALC_INTERVAL) {
        uint32_t pulses = pulseCount - lastPulseCount;
        currentRpm = (pulses * 60000.0) / (RPM_CALC_INTERVAL * ENCODER_PPR);
        if (pidOutput < 0) currentRpm = -currentRpm;  // Apply direction to RPM
        lastPulseCount = pulseCount;
        lastRpmCalc = now;
        pidInput = currentRpm;
    }
}

void Motor::setRpm(double rpm) {
    rpm = constrain(rpm, -MAX_RPM, MAX_RPM);
    pidSetpoint = rpm;
}

void Motor::update() {
    calculateRpm();
    if (pidController.Compute()) {
        applyOutput();
    }
}

void Motor::applyOutput() {
    int pwm = abs(pidOutput);
    if (pidOutput >= 0) {
        analogWrite(in2Pin, 0);
        analogWrite(in1Pin, pwm);
    } else {
        analogWrite(in1Pin, 0);
        analogWrite(in2Pin, pwm);
    }
}

void Motor::stop() {
    pidSetpoint = 0;
    analogWrite(in1Pin, 0);
    analogWrite(in2Pin, 0);
}
