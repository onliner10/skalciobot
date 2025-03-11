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

    // Calculate base steering from side sensors
    float steering = calculateSteering(left, right);
    
    // Handle front obstacle
    if (front < MAX_TURN_DISTANCE) {
        float frontMultiplier = calculateFrontMultiplier(front);
        
        // Only randomize direction if we're exactly straight
        if (steering == 0) {
            logger.info("Front obstacle detected while straight, selecting random direction", 
                       LogContext::Navigation);
            steering = (random(2) ? 0.5f : -0.5f);
        }
        
        steering *= frontMultiplier;
        steering = constrain(steering, -1.0f, 1.0f);
        
        logger.debug("Front obstacle influence - multiplier: " + String(frontMultiplier) + 
                    ", final steering: " + String(steering), LogContext::Navigation);
    }

    motors.setSteering(steering);
    motors.setSpeedPercent(calculateTargetSpeed(front));
}

float RobotLogic::calculateSteering(uint16_t left, uint16_t right) {
    // Calculate normalized distances (0 = very close, 1 = far)
    float leftNorm = (float)left / MAX_TURN_DISTANCE;
    float rightNorm = (float)right / MAX_TURN_DISTANCE;
    
    // Calculate turn strengths (inverse of distance)
    float leftTurn = 1.0f - constrain(leftNorm, 0.0f, 1.0f);
    float rightTurn = 1.0f - constrain(rightNorm, 0.0f, 1.0f);
    
    // Apply exponential response curve
    leftTurn = pow(leftTurn, TURN_EXPONENT * 200);  // Multiplier to make exponent more effective
    rightTurn = pow(rightTurn, TURN_EXPONENT * 200);
    
    // Return difference (positive = turn right, negative = turn left)
    return leftTurn - rightTurn;
}

float RobotLogic::calculateFrontMultiplier(uint16_t front) {
    if (front >= MAX_TURN_DISTANCE) return 1.0f;
    
    float normalizedDist = (float)(front - MIN_TURN_DISTANCE) / 
                          (float)(MAX_TURN_DISTANCE - MIN_TURN_DISTANCE);
    return 1.0f + ((FRONT_MULTIPLIER_MAX - 1.0f) * (1.0f - constrain(normalizedDist, 0.0f, 1.0f)));
}

int RobotLogic::calculateTargetSpeed(uint16_t front) {
    return map(
        constrain(front, MIN_SPEED_DISTANCE, MAX_SPEED_DISTANCE),
        MIN_SPEED_DISTANCE,
        MAX_SPEED_DISTANCE,
        0,   // Half speed (50%)
        100   // Full speed (100%)
    );
}
