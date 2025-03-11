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
    speedPercent = (pwm / 255.0f) * 100.0f;
    updatePID();
}

void MotorController::setSteering(float steering) {
    targetSteeringRatio = constrain(steering, -1.0f, 1.0f);
    updatePID();
}

void MotorController::setSpeedPercent(float percent) {
    speedPercent = constrain(percent, -100.0f, 100.0f);
    updatePID();
}

void MotorController::update() {
    if (checkFault()) {
        stop();
        return;
    }

    unsigned long now = millis();
    if (now - lastPidUpdate >= PID_UPDATE_INTERVAL) {
        updatePID();
        lastPidUpdate = now;
    }
}

float MotorController::calculateCurrentSteeringRatio() const {
    float leftSpeed = leftMotor.getCurrentSpeed();
    float rightSpeed = rightMotor.getCurrentSpeed();
    
    logger.debug("Speed (pulses) - L:" + String(leftSpeed) + " R:" + String(rightSpeed), LogContext::Motor);
    
    // Avoid division by zero and very small values
    if (abs(leftSpeed) < 2.0f && abs(rightSpeed) < 2.0f) {
        return 0.0f;
    }

    // Calculate normalized difference between motors
    // Positive ratio = turning right (right motor slower)
    float avgSpeed = (abs(leftSpeed) + abs(rightSpeed)) / 2.0f;
    if (avgSpeed < 2.0f) return 0.0f;
    
    // Adjust ratio calculation based on direction of movement
    float ratio;
    if (speedPercent >= 0) {
        ratio = (leftSpeed - rightSpeed) / (2.0f * avgSpeed);
    } else {
        ratio = (rightSpeed - leftSpeed) / (2.0f * avgSpeed);
    }
    ratio = constrain(ratio, -1.0f, 1.0f);
    
    logger.debug("Steering ratio: " + String(ratio), LogContext::Motor);
    return ratio;
}

void MotorController::updatePID() {
    if (!state.isEnabled() || speedPercent == 0) {
        stop();
        return;
    }

    float currentRatio = calculateCurrentSteeringRatio();
    float error = targetSteeringRatio - currentRatio;
    
    float deltaTime = PID_UPDATE_INTERVAL / 1000.0f;
    
    // Reset integral when changing direction
    if (signbit(error) != signbit(lastSteeringError)) {
        steeringIntegral = 0;
    }
    
    steeringIntegral += error * deltaTime;
    steeringIntegral = constrain(steeringIntegral, -STEERING_INTEGRAL_LIMIT, STEERING_INTEGRAL_LIMIT);
    
    float derivative = (error - lastSteeringError) / deltaTime;
    
    // Calculate correction but make it more aggressive
    float correction = (KP * error) + (KI * steeringIntegral) + (KD * derivative);
    correction = constrain(correction, -1.0f, 1.0f);
    
    logger.debug("PID components - P:" + String(KP * error) + 
                " I:" + String(KI * steeringIntegral) + 
                " D:" + String(KD * derivative) + 
                " Total:" + String(correction), LogContext::Motor);
    
    // Calculate base PWM from speed percent
    float basePwm = (speedPercent / 100.0f) * 255.0f;
    float leftPwm = basePwm;
    float rightPwm = basePwm;
    
    leftPwm *= (1.0f + correction);
    rightPwm *= (1.0f - correction);
    
    // Maintain the sign of the original speed
    if (speedPercent < 0) {
        leftPwm = -leftPwm;
        rightPwm = -rightPwm;
    }

    logger.debug("PWM - L:" + String(leftPwm) + " R:" + String(rightPwm) + 
                " Correction:" + String(correction), LogContext::Motor);
    
    // Enable motors if needed
    if (leftPwm != 0 || rightPwm != 0) {
        enable();
        delay(1);
    }
    
    applyMotorOutputs(leftPwm, rightPwm);
    lastSteeringError = error;
}

bool MotorController::shouldUpdatePwm(float leftPwm, float rightPwm) const {
    // Check if enough time has passed
    if (millis() - lastPwmUpdate < MOTOR_PWM_MIN_UPDATE_INTERVAL) {
        return false;
    }
    
    // Check if change is significant enough for either motor
    if (abs(leftPwm - lastLeftPwm) < MOTOR_PWM_MIN_CHANGE &&
        abs(rightPwm - lastRightPwm) < MOTOR_PWM_MIN_CHANGE) {
        return false;
    }
    
    return true;
}

void MotorController::applyMotorOutputs(float leftPwm, float rightPwm) {
    if (!shouldUpdatePwm(leftPwm, rightPwm)) {
        return;
    }
    
    // Apply motor scaling factors
    leftPwm *= leftMotorScale;
    rightPwm *= rightMotorScale;
    
    // Smooth the PWM changes
    float smoothedLeftPwm = (leftPwm * PWM_SMOOTHING) + (lastOutputLeftPwm * (1.0f - PWM_SMOOTHING));
    float smoothedRightPwm = (rightPwm * PWM_SMOOTHING) + (lastOutputRightPwm * (1.0f - PWM_SMOOTHING));
    
    leftMotor.setPwm(int(constrain(smoothedLeftPwm, -255.0f, 255.0f)));
    rightMotor.setPwm(int(constrain(smoothedRightPwm, -255.0f, 255.0f)));
    
    lastLeftPwm = leftPwm;
    lastRightPwm = rightPwm;
    lastOutputLeftPwm = smoothedLeftPwm;
    lastOutputRightPwm = smoothedRightPwm;
    lastPwmUpdate = millis();
}

void MotorController::stop() {
    speedPercent = 0;
    targetSteeringRatio = 0;
    steeringIntegral = 0;
    lastSteeringError = 0;
    lastOutputLeftPwm = 0;
    lastOutputRightPwm = 0;
    leftMotor.stop();
    rightMotor.stop();
    disable();
}

bool MotorController::checkFault() {
    // Fault is active LOW (open-drain output)
    return digitalRead(faultPin) == LOW;
}

void MotorController::sleep(bool enable) {
    digitalWrite(sleepPin, enable ? LOW : HIGH);
}

void MotorController::test() {
    if (!state.isEnabled()) {
        return;
    }
    
    enable();
    delay(1);
    
    // Test forward
    setSpeedPercent(50);  // Half speed forward
    delay(1000);
    
    // Test backward
    setSpeedPercent(-50); // Half speed backward
    delay(1000);
    
    stop();
}

void MotorController::calibrateMotors() {
    if (!state.isEnabled()) return;
    
    logger.info("Starting motor calibration...", LogContext::Motor);
    enable();
    delay(1);
    
    // Reset speed buffers
    leftMotor.stop();
    rightMotor.stop();
    delay(100);
    
    // Run motors at calibration speed
    setSpeedPercent(MOTOR_CALIBRATION_SPEED);
    delay(MOTOR_CALIBRATION_TIME);
    
    // Get average speeds
    float leftSpeed = leftMotor.getCurrentSpeed();
    float rightSpeed = rightMotor.getCurrentSpeed();
    
    // Calculate scaling factors
    if (leftSpeed > rightSpeed) {
        leftMotorScale = rightSpeed / leftSpeed;
        rightMotorScale = 1.0f;
    } else {
        rightMotorScale = leftSpeed / rightSpeed;
        leftMotorScale = 1.0f;
    }
    
    logger.info("Calibration complete - L:" + String(leftMotorScale) + 
                " R:" + String(rightMotorScale), LogContext::Motor);
    
    stop();
}
