#include "StuckDetector.h"

void StuckDetector::update() {
    if (millis() - lastUpdate < STUCK_UPDATE_INTERVAL) return;
    lastUpdate = millis();
    
    // Get current readings
    uint16_t currentReadings[3] = {
        sensors.getFrontDistance(),
        sensors.getLeftDistance(),
        sensors.getRightDistance()
    };
    
    // Calculate and store deltas
    for (int i = 0; i < 3; i++) {
        deltaHistory[historyIndex][i] = (int16_t)currentReadings[i] - (int16_t)previousReadings[i];
        previousReadings[i] = currentReadings[i];
    }
    
    historyIndex = (historyIndex + 1) % STUCK_HISTORY_SIZE;
    
    // Mark as initialized after collecting enough samples
    if (!isInitialized && historyIndex >= STUCK_HISTORY_SIZE - 1) {
        isInitialized = true;
    }
}

float StuckDetector::calculateDeltaStdDev(size_t sensorIndex) const {
    float sum = 0.0f;
    float mean = 0.0f;
    
    // Calculate mean of deltas
    for (size_t i = 0; i < STUCK_HISTORY_SIZE; i++) {
        mean += deltaHistory[i][sensorIndex];
    }
    mean /= STUCK_HISTORY_SIZE;
    
    // Calculate standard deviation of deltas
    for (size_t i = 0; i < STUCK_HISTORY_SIZE; i++) {
        float diff = deltaHistory[i][sensorIndex] - mean;
        sum += diff * diff;
    }
    
    return sqrt(sum / STUCK_HISTORY_SIZE);
}

float StuckDetector::getSpeedDependentThreshold() const {
    // Get current PWM values to calculate speed
    int16_t leftPwm = abs(leftMotor.getCurrentPwm());
    int16_t rightPwm = abs(rightMotor.getCurrentPwm());
    
    // Use the average PWM as a measure of speed
    float avgPwm = (leftPwm + rightPwm) / 2.0f;
    
    // Get maximum possible PWM value based on resolution
    float maxPwm = (1 << MOTOR_PWM_RESOLUTION) - 1;
    
    // Calculate normalized speed between 0 and 1
    float normalizedSpeed = avgPwm / maxPwm;
    
    // Apply a function that decreases threshold as speed increases
    // At low speeds (close to 0), we use STUCK_MIN_STDDEV_LOW_SPEED
    // At high speeds (close to 1), we use STUCK_MIN_STDDEV_HIGH_SPEED
    
    // If motors are stopped, use a high threshold
    if (normalizedSpeed < 0.01f) {
        return STUCK_MIN_STDDEV_HIGH_SPEED; // Effectively disable stuck detection when stopped
    }
    
    // Linear interpolation between high and low thresholds
    return STUCK_MIN_STDDEV_LOW_SPEED + 
           (STUCK_MIN_STDDEV_HIGH_SPEED - STUCK_MIN_STDDEV_LOW_SPEED) * 
           (1.0f - normalizedSpeed);
}

bool StuckDetector::isEncoderStuck() const {
    if (leftMotor.getCurrentPwm() == 0 && rightMotor.getCurrentPwm() == 0) {
        return false;  // Not stuck if motors are stopped
    }
    
    return leftMotor.getTimeSinceLastPulse() > STUCK_ENCODER_TIME &&
           rightMotor.getTimeSinceLastPulse() > STUCK_ENCODER_TIME;
}

bool StuckDetector::isDistanceStuck() const {
    // Get a dynamic threshold based on current speed
    float threshold = getSpeedDependentThreshold();
    
    // Count sensors with low standard deviation
    int stuckSensors = 0;
    
    // Check each sensor
    if (calculateDeltaStdDev(0) < threshold) stuckSensors++; // Front
    if (calculateDeltaStdDev(1) < threshold) stuckSensors++; // Left
    if (calculateDeltaStdDev(2) < threshold) stuckSensors++; // Right
    
    // Return true if at least 2 out of 3 sensors show minimal changes
    return stuckSensors >= 2;
}

bool StuckDetector::isStuck() const {
    // Don't detect stuck during initialization or cooldown period
    if (!isInitialized || (millis() - lastBackupTime < STUCK_BACKUP_COOLDOWN)) {
        return false;
    }
    
    return isEncoderStuck() || isDistanceStuck();
}
