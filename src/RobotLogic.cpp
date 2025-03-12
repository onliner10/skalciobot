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
    float steering = (0.3 * rightClearance + (rightClearance, 2)) - (0.3 * leftClearance + (leftClearance, 2));
    
    // In near-deadzone situations (almost equal side clearances), 
    // pick turn direction based on smallest noise difference
    if (abs(steering) < 0.05f) {
        steering = (left < right) ? -0.05f : 0.05f;
    }
    
    // Get normalized front distance influence (0 = far, 1 = close)
    // Start influence even earlier but make it more gradual
    float frontInfluence = 1.0f - constrain((float)front / (MAX_SENSOR_DISTANCE ), 0.0f, 1.0f);
    
    // Reduce multiplier from 0.5 to 0.35 for gentler influence
    steering *= (1.0f + 4 * pow(frontInfluence, 4));  // Max 1.35x amplification when front is blocked
    
    return constrain(steering, -1.0f, 1.0f);
}

int RobotLogic::calculateTargetSpeed(uint16_t front) {
    // Find minimum distance from all sensors
    uint16_t minDistance = min(front, min(sensors.getLeftDistance(), sensors.getRightDistance()));
    
    if (minDistance < SPEED_THRESHOLD_MM) {
        // Linear decrease below threshold
        float speedFactor = (float)minDistance / SPEED_THRESHOLD_MM;
        speedFactor = pow(speedFactor, 3);  // Square for more aggressive slowdown
        return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT) * 0.5f);
    } else {
        // Quick exponential increase above threshold
        float overshoot = (float)(minDistance - SPEED_THRESHOLD_MM) / (MAX_SENSOR_DISTANCE - SPEED_THRESHOLD_MM);
        float speedFactor = 0.5f + (1.0f - exp(-overshoot * 3)) * 0.5f;
        return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT));
    }
}
