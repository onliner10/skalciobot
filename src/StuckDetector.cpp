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

bool StuckDetector::isEncoderStuck() const {
    if (leftMotor.getCurrentPwm() == 0 && rightMotor.getCurrentPwm() == 0) {
        return false;  // Not stuck if motors are stopped
    }
    
    return leftMotor.getTimeSinceLastPulse() > STUCK_ENCODER_TIME &&
           rightMotor.getTimeSinceLastPulse() > STUCK_ENCODER_TIME;
}

bool StuckDetector::isDistanceStuck() const {
    // Count sensors with low standard deviation
    int stuckSensors = 0;
    
    // Check each sensor
    if (calculateDeltaStdDev(0) < STUCK_MIN_STDDEV) stuckSensors++; // Front
    if (calculateDeltaStdDev(1) < STUCK_MIN_STDDEV) stuckSensors++; // Left
    if (calculateDeltaStdDev(2) < STUCK_MIN_STDDEV) stuckSensors++; // Right
    
    // Return true if at least 2 out of 3 sensors show minimal changes
    return stuckSensors >= 2;
}

bool StuckDetector::isStuck() const {
    return isEncoderStuck() || isDistanceStuck();
}
