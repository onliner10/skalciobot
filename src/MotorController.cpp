#include "MotorController.h"

MotorController::MotorController(Motor& left, Motor& right, int slp, int flt)
    : leftMotor(left), rightMotor(right), sleepPin(slp), faultPin(flt) {}

void MotorController::begin() {
    pinMode(sleepPin, OUTPUT);
    pinMode(faultPin, INPUT_PULLUP);
    
    digitalWrite(sleepPin, HIGH);  // Enable motors
    leftMotor.begin();
    rightMotor.begin();
}

void MotorController::setRpm(double rpm) {
    currentRpm = constrain(rpm, -MAX_RPM, MAX_RPM);
    updateMotors();
}

void MotorController::setSteering(float steering) {
    currentSteering = constrain(steering, -1.0f, 1.0f);
    updateMotors();
}

void MotorController::updateMotors() {
    // Calculate individual motor RPMs based on steering
    float leftMultiplier = 1.0f - (currentSteering > 0 ? currentSteering : 0);
    float rightMultiplier = 1.0f + (currentSteering < 0 ? currentSteering : 0);
    
    double leftRpm = currentRpm * leftMultiplier;
    double rightRpm = currentRpm * rightMultiplier;
    
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
    currentRpm = 0;
    currentSteering = 0;
    leftMotor.stop();
    rightMotor.stop();
}

bool MotorController::checkFault() {
    return digitalRead(faultPin) == LOW;
}

void MotorController::sleep(bool enable) {
    digitalWrite(sleepPin, enable ? LOW : HIGH);
}
