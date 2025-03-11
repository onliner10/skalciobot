#include "RobotLogic.h"

void RobotLogic::begin() {
    motors.begin();
    sensors.begin();
}

void RobotLogic::update() {
    if (!state.isAuto()) {
        return;  // Only run autonomous logic in Auto mode
    }

    uint16_t front = sensors.getFrontDistance();
    uint16_t left = sensors.getLeftDistance();
    uint16_t right = sensors.getRightDistance();

    float steering = calculateSteering(left, right);
    
    // If front obstacle detected and steering is too small, force minimum steering
    if (front < FRONT_START_DISTANCE && abs(steering) < MIN_FRONT_STEERING) {
        steering = (steering >= 0 ? MIN_FRONT_STEERING : -MIN_FRONT_STEERING);
    }
    
    // Apply front multiplier and constrain
    steering *= calculateFrontMultiplier(front);
    steering = constrain(steering, -1.0f, 1.0f);
    
    motors.setSteering(steering);
    motors.setSpeedPercent(calculateTargetSpeed(front));
}

float RobotLogic::calculateSteering(uint16_t left, uint16_t right) {
    // Convert to 0-1 range where 1 means closest to obstacle
    float leftInfluence = 1.0f - ((float)left / MAX_SENSOR_DISTANCE);
    float rightInfluence = 1.0f - ((float)right / MAX_SENSOR_DISTANCE);
    
    // Apply gentle curve: f(x) = x + (x^2)*0.4
    // This creates a curve that's between linear and quadratic
    leftInfluence = leftInfluence + (leftInfluence * leftInfluence) * 0.35f;
    rightInfluence = rightInfluence + (rightInfluence * rightInfluence) * 0.35f;
    
    return leftInfluence - rightInfluence;  // Positive = turn right, negative = turn left
}

float RobotLogic::calculateFrontMultiplier(uint16_t front) {
    if (front >= FRONT_START_DISTANCE) return 1.0f;
    if (front <= FRONT_MIN_DISTANCE) return FRONT_AMPLIFICATION;
    
    // Calculate normalized distance between min and start points (0.0 - 1.0)
    float normalized = (float)(front - FRONT_MIN_DISTANCE) / 
                      (float)(FRONT_START_DISTANCE - FRONT_MIN_DISTANCE);
    
    // Apply exponential curve (use normalized^2 for faster growth near obstacles)
    float factor = 1.0f - (normalized * normalized);
    
    // Calculate final multiplier
    return 1.0f + (factor * (FRONT_AMPLIFICATION - 1.0f));
}

int RobotLogic::calculateTargetSpeed(uint16_t front) {
    // Linear speed control based on front distance
    float speedFactor = (float)front / MAX_SENSOR_DISTANCE;
    return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT));
}
