#include "MotorController.h"

MotorController::MotorController(
    int steeringM1, int steeringM2,
    int driveM1, int driveM2,
    int mFault, int mSleep, int mFlt
) : steeringMotor1(steeringM1), steeringMotor2(steeringM2),
    driveMotor1(driveM1), driveMotor2(driveM2),
    motorFault(mFault), motorSleep(mSleep), motorFlt(mFlt),
    faultCount(0), lastFaultTime(0) {}

void MotorController::begin() {
    pinMode(steeringMotor1, OUTPUT);
    pinMode(steeringMotor2, OUTPUT);
    pinMode(driveMotor1, OUTPUT);
    pinMode(driveMotor2, OUTPUT);
    pinMode(motorFlt, INPUT_PULLUP);  // Only initialize the FLT pin
    pinMode(motorSleep, OUTPUT);
    
    // Configure PWM for ESP32
    analogWriteResolution(PWM_RESOLUTION);
    analogWriteFrequency(PWM_FREQUENCY);  // Set global PWM frequency
    
    digitalWrite(motorSleep, HIGH);
}

void MotorController::setSpeed(int speed) {
    if (checkFault()) {
        if (!attemptFaultRecovery()) {
            stop();
            return;
        }
    }

    speed = constrain(speed, -100, 100);
    // Apply deadband
    if (abs(speed) < PWM_DEADBAND) {
        speed = 0;
    }
    
    currentSpeed = speed;
    
    // Map speed more aggressively - minimum 50% power when moving
    if (speed > 0) {
        analogWrite(driveMotor2, 0);  // Set one direction first
        delayMicroseconds(50);        // Small delay to prevent shoot-through
        analogWrite(driveMotor1, map(speed, 0, 100, 128, 255));
    } else if (speed < 0) {
        analogWrite(driveMotor1, 0);  // Set one direction first
        delayMicroseconds(50);        // Small delay to prevent shoot-through
        analogWrite(driveMotor2, map(-speed, 0, 100, 128, 255));
    } else {
        analogWrite(driveMotor1, 0);
        analogWrite(driveMotor2, 0);
    }
}

void MotorController::setSteering(SteeringDirection direction) {
    if (checkFault()) {
        if (!attemptFaultRecovery()) {
            stop();
            return;
        }
    }

    currentSteering = direction;
    
    switch(direction) {
        case STRAIGHT:
            analogWrite(steeringMotor1, 0);
            analogWrite(steeringMotor2, 0);
            break;
        case LEFT:
            analogWrite(steeringMotor2, 0);
            delayMicroseconds(50);
            analogWrite(steeringMotor1, 255);  // Full power for turning
            break;
        case RIGHT:
            analogWrite(steeringMotor1, 0);
            delayMicroseconds(50);
            analogWrite(steeringMotor2, 255);  // Full power for turning
            break;
    }
}

void MotorController::stop() {
    analogWrite(steeringMotor1, 0);
    analogWrite(steeringMotor2, 0);
    analogWrite(driveMotor1, 0);
    analogWrite(driveMotor2, 0);
    currentSpeed = 0;
    currentSteering = STRAIGHT;
}

bool MotorController::checkFault() {
    return digitalRead(motorFlt) == LOW;  // Only check FLT pin
}

bool MotorController::attemptFaultRecovery() {
    if (!isRecoverable()) {
        return false;
    }
    
    // Implement reset sequence
    digitalWrite(motorSleep, LOW);  // Put driver to sleep
    delay(FAULT_RESET_DELAY);      // Wait for driver to reset
    digitalWrite(motorSleep, HIGH); // Wake driver
    delay(FAULT_RESET_DELAY);      // Wait for driver to stabilize
    
    faultCount++;
    lastFaultTime = millis();
    
    return !checkFault();  // Return true if fault cleared
}

void MotorController::resetFaultCount() {
    faultCount = 0;
}
