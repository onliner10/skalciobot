#include "RobotLogic.h"

void RobotLogic::begin() {
    motors.begin();
    sensors.begin();
}

void RobotLogic::update() {
    if (!state.isAuto()) {
        return;  // Only run autonomous logic in Auto mode
    }

    if (!sensors.hasNewMeasurements()) {
        return;  // Only update when new measurements are available
    }

    uint16_t front = sensors.getFrontDistance();
    uint16_t left = sensors.getLeftDistance();
    uint16_t right = sensors.getRightDistance();

    float steering = calculateSteering(left, right);
    motors.setSteering(steering);
    motors.setSpeedPercent(calculateTargetSpeed(front));
    
    sensors.clearNewMeasurementsFlag();
}

float RobotLogic::calculateSteering(uint16_t left, uint16_t right) {
    // Normalize distances to 0-1 range (0 = obstacle close, 1 = clear path)
    float leftClearance = constrain((float)left / MAX_SENSOR_DISTANCE, 0.0f, 1.0f);
    float rightClearance = constrain((float)right / MAX_SENSOR_DISTANCE, 0.0f, 1.0f);
    
    // Calculate steering
    // rightClearance - leftClearance means:
    // - If right is clearer than left (rightClearance > leftClearance), go right (positive)
    // - If left is clearer than right (leftClearance > rightClearance), go left (negative)
    float steering = rightClearance - leftClearance;
    
    // Add small deadzone
    if (abs(steering) < 0.05f) return 0.0f;
    
    return steering;
}

int RobotLogic::calculateTargetSpeed(uint16_t front) {
    // Linear speed control based on front distance
    float speedFactor = (float)front / MAX_SENSOR_DISTANCE;
    return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT));
}
