#include "MotorController.h"

MotorController::MotorController(Motor& left, Motor& right, int flt, RobotState& s, Logger& log)
    : leftMotor(left), rightMotor(right), faultPin(flt), state(s), logger(log) {}

void MotorController::begin() {
    pinMode(faultPin, INPUT_PULLUP);  // Add internal pullup for open-drain output
    
    leftMotor.begin();
    rightMotor.begin();
}

void MotorController::setSteering(float steering) {
    targetSteeringRatio = constrain(steering, -1.0f, 1.0f);
}

void MotorController::setSpeedPercent(float percent) {
    speedPercent = constrain(percent, -100.0f, 100.0f);
}

float MotorController::calculateCurrentSteeringRatio() const {
    float leftSpeed = leftMotor.getCurrentSpeed();
    float rightSpeed = rightMotor.getCurrentSpeed();
    
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
    
    return ratio;
}

// Rename updatePID to update for consistency
void MotorController::update() {
    unsigned long now = millis();
    if (now - lastPidUpdate < STEERING_PID_INTERVAL) {
        return;  // Not time for PID update yet
    }
    lastPidUpdate = now;

    // Skip normal updates if in backup mode
    if (backupModeActive) {
        return;
    }

    if (!state.isEnabled() || speedPercent == 0) {
        stop();
        return;
    }

    if (checkFault()) {
        stop();
        return;
    }

    float currentRatio = calculateCurrentSteeringRatio();
    float error = targetSteeringRatio - currentRatio;
    
    // Reset integral when changing direction
    if (signbit(error) != signbit(lastSteeringError)) {
        steeringIntegral = 0;
    }
    
    steeringIntegral += error * (STEERING_PID_INTERVAL / 1000.0f);
    steeringIntegral = constrain(steeringIntegral, -STEERING_INTEGRAL_LIMIT, STEERING_INTEGRAL_LIMIT);
    
    float derivative = (error - lastSteeringError) / (STEERING_PID_INTERVAL / 1000.0f);
    
    float correction = (KP * error) + (KI * steeringIntegral) + (KD * derivative);
    correction = constrain(correction, -1.0f, 1.0f);
    
    float basePwm = (speedPercent / 100.0f) * ((1 << MOTOR_PWM_RESOLUTION) - 1);
    float leftPwm = basePwm * (1.0f + correction) * leftMotorScale;
    float rightPwm = basePwm * (1.0f - correction) * rightMotorScale;
    
    if (speedPercent < 0) {
        leftPwm = -leftPwm;
        rightPwm = -rightPwm;
    }

    // Apply PWM directly
    leftMotor.setPwm(leftPwm);
    rightMotor.setPwm(rightPwm);
    
    lastSteeringError = error;
}

void MotorController::stop() {
    speedPercent = 0;
    targetSteeringRatio = 0;
    steeringIntegral = 0;
    lastSteeringError = 0;
    leftMotor.stop();
    rightMotor.stop();
}

bool MotorController::checkFault() {
    // Fault is active LOW (open-drain output)
    return digitalRead(faultPin) == LOW;
}

void MotorController::test() {
    if (!state.isEnabled()) {
        return;
    }
    
    const int testPwm = 1023;  // Half of 10-bit range (1024)
    
    // Test forward
    leftMotor.setPwm(testPwm);
    rightMotor.setPwm(testPwm);
    delay(2000);
    
    // Test backward
    leftMotor.setPwm(-testPwm);
    rightMotor.setPwm(-testPwm);
    delay(2000);
    
    stop();
}

void MotorController::calibrateMotors() {
    if (!state.isEnabled()) return;
    
    logger.info("Starting motor calibration...", LogContext::Motor);
    delay(1);
    
    // Reset speed buffers
    leftMotor.stop();
    rightMotor.stop();
    delay(100);
    
    // Run motors at calibration PWM
    const int calibrationPwm = (1 << MOTOR_PWM_RESOLUTION) / 2;  // 50% of max PWM
    leftMotor.setPwm(calibrationPwm);
    rightMotor.setPwm(calibrationPwm);
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
