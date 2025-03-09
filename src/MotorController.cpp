#include "MotorController.h"

MotorController::MotorController(Motor& left, Motor& right, int slp, int flt)
    : leftMotor(left), rightMotor(right), sleepPin(slp), faultPin(flt) {}

void MotorController::begin() {
    pinMode(sleepPin, OUTPUT);
    pinMode(faultPin, INPUT);
    digitalWrite(sleepPin, LOW);  // Start with drivers disabled
    leftMotor.begin();
    rightMotor.begin();
}

void MotorController::setRpm(double rpm) {
    currentRpm = constrain(rpm, -MAX_RPM, MAX_RPM);
    // Enable drivers BEFORE setting RPM, disable only when explicitly stopped
    if (currentRpm != 0) {
        digitalWrite(sleepPin, HIGH);
        delay(1);  // Brief delay to let driver enable
    }
    updateMotors();
}

void MotorController::setSteering(float steering) {
    currentSteering = constrain(steering, -1.0f, 1.0f);
    updateMotors();
}

void MotorController::updateMotors() {
    // When steering is zero, both motors run at full target RPM
    // When turning, outer wheel maintains speed while inner wheel reduces
    double leftRpm = currentRpm;
    double rightRpm = currentRpm;
    
    if (currentSteering > 0) {  // Turning right
        leftRpm = currentRpm;   // Outer wheel maintains speed
        rightRpm = currentRpm * (1.0f - currentSteering);  // Inner wheel reduces
    } else if (currentSteering < 0) {  // Turning left
        leftRpm = currentRpm * (1.0f + currentSteering);   // Inner wheel reduces
        rightRpm = currentRpm;  // Outer wheel maintains speed
    }
    
    leftMotor.setRpm(leftRpm);
    rightMotor.setRpm(rightRpm);
}

void MotorController::update() {
    if (checkFault()) {
        stop();
        return;
    }
    leftMotor.update();
    rightMotor.update();
}

void MotorController::stop() {
    // Stop motors first
    leftMotor.stop();
    rightMotor.stop();
    currentRpm = 0;
    currentSteering = 0;
    // Disable drivers after stopping
    digitalWrite(sleepPin, LOW);
}

bool MotorController::checkFault() {
    return digitalRead(faultPin) == LOW;
}

void MotorController::sleep(bool enable) {
    digitalWrite(sleepPin, enable ? LOW : HIGH);
}
