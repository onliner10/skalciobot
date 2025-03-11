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

    float steering = calculateSteering(left, right, front); // Pass front distance
    motors.setSteering(steering);
    motors.setSpeedPercent(calculateTargetSpeed(front));
    
    sensors.clearNewMeasurementsFlag();
}

float RobotLogic::calculateSteering(uint16_t left, uint16_t right, uint16_t front) {
    // Normalize distances to 0-1 range (0 = obstacle close, 1 = clear path)
    float leftClearance = constrain((float)left / MAX_SENSOR_DISTANCE, 0.0f, 1.0f);
    float rightClearance = constrain((float)right / MAX_SENSOR_DISTANCE, 0.0f, 1.0f);
    
    // Calculate base steering from side sensors
    // Use square root to make the response more gentle
    float steering = sqrt(rightClearance) - sqrt(leftClearance);
    
    // In near-deadzone situations (almost equal side clearances), 
    // pick turn direction based on smallest noise difference
    if (abs(steering) < 0.05f) {
        steering = (left < right) ? -0.05f : 0.05f;
    }
    
    // Get normalized front distance influence (0 = far, 1 = close)
    // Start influence even earlier but make it more gradual
    float frontInfluence = 1.0f - constrain((float)front / (MAX_SENSOR_DISTANCE * 0.9f), 0.0f, 1.0f);
    
    // Reduce multiplier from 0.5 to 0.35 for gentler influence
    steering *= (1.0f + frontInfluence * 0.35f);  // Max 1.35x amplification when front is blocked
    
    return constrain(steering, -1.0f, 1.0f);
}

int RobotLogic::calculateTargetSpeed(uint16_t front) {
    // Linear speed control based on front distance
    // float speedFactor = (float)front / MAX_SENSOR_DISTANCE;
    // return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT));
    return MIN_SPEED_PERCENT;
}
