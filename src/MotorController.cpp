#include "MotorController.h"

MotorController::MotorController(Motor& left, Motor& right, int slp, int flt)
    : leftMotor(left), rightMotor(right), sleepPin(slp), faultPin(flt) {}

void MotorController::begin() {
    pinMode(sleepPin, OUTPUT);
    pinMode(faultPin, INPUT_PULLUP);  // Add internal pullup for open-drain output
    
    // Start with motors disabled
    digitalWrite(sleepPin, LOW);
    
    leftMotor.begin();
    rightMotor.begin();
}

void MotorController::setPwm(int pwm) {
    currentPwm = constrain(pwm, -255, 255);
    if (currentPwm != 0) {
        enable();  // Enable motors when setting non-zero PWM
        delay(1);
    }
    updateMotors();
}

void MotorController::setSteering(float steering) {
    currentSteering = constrain(steering, -1.0f, 1.0f);
    updateMotors();
}

void MotorController::updateMotors() {
    if (isFault()) {
        stop();
        return;
    }

    int leftPwm = currentPwm;
    int rightPwm = currentPwm;
    
    // Steering is already in correct direction: -1 (left) to 1 (right)
    if (currentSteering > 0) {  // Turning right
        rightPwm = int(rightPwm * (1.0f - currentSteering));  // Reduce inner wheel
    } else if (currentSteering < 0) {  // Turning left
        leftPwm = int(leftPwm * (1.0f + currentSteering));    // Reduce inner wheel
    }
    
    leftMotor.setPwm(leftPwm);
    rightMotor.setPwm(rightPwm);
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
    leftMotor.stop();
    rightMotor.stop();
    currentPwm = 0;
    currentSteering = 0;
    disable();  // Disable motors after stopping
}

bool MotorController::checkFault() {
    // Fault is active LOW (open-drain output)
    return digitalRead(faultPin) == LOW;
}

void MotorController::sleep(bool enable) {
    digitalWrite(sleepPin, enable ? LOW : HIGH);
}
