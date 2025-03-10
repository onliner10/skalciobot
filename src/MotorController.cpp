#include "MotorController.h"

MotorController::MotorController(Motor& left, Motor& right, int slp, int flt, RobotState& s, Logger& log)
    : leftMotor(left), rightMotor(right), sleepPin(slp), faultPin(flt), state(s), logger(log) {}

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

void MotorController::setRPM(float rpm) {
    currentRPM = constrain(rpm, -MAX_RPM, MAX_RPM);
    if (currentRPM != 0) {
        enable();
        delay(1);
    }
    updateMotors();
}

void MotorController::updateMotors() {
    if (!state.isEnabled() || isFault()) {
        stop();
        return;
    }

    // Calculate base RPM for each motor
    float targetLeft = currentRPM;
    float targetRight = currentRPM;
    
    // Apply steering - positive steering = turn right (reduce right motor)
    if (currentSteering > 0) {  // Turning right
        targetRight *= (1.0f - currentSteering);
    } else if (currentSteering < 0) {  // Turning left
        targetLeft *= (1.0f + currentSteering);
    }
    
    // Apply RPM to both motors
    if (targetLeft != 0) enable();
    if (targetRight != 0) enable();
    
    leftMotor.setRPM(targetLeft);
    rightMotor.setRPM(targetRight);
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
