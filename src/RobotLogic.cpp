#include "RobotLogic.h"

void RobotLogic::begin() {
    motors.begin();
    sensors.begin();
}

void RobotLogic::update() {
    if (!state.isAuto()) {
        return;  // Only run autonomous logic in Auto mode
    }

    // Check if stuck (no pulses for a while)
    if (motors.getLeftTimeSinceLastPulse() > 1000 && 
        motors.getRightTimeSinceLastPulse() > 1000) {
        handleStuckState();
        return;
    }

    if (isPerformingRecovery) {
        updateRecoveryManeuver();
        return;
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
    motors.setPwm(calculateTargetSpeed(front));
}

void RobotLogic::handleStuckState() {
    logger.warning("Robot stuck - starting recovery", LogContext::Navigation);
    isPerformingRecovery = true;
    recoveryStartTime = millis();
    backupDuration = random(BACKUP_MIN_TIME, BACKUP_MAX_TIME);
    turnDuration = random(TURN_MIN_TIME, TURN_MAX_TIME);
    recoveryPhase = RecoveryPhase::Backing;
    
    // Alternate between forward and backward recovery
    int recoverySpeed = lastRecoveryWasBackward ? 100 : -100;
    lastRecoveryWasBackward = !lastRecoveryWasBackward;
    
    // Use opposite of current steering direction for recovery
    float recoveryDirection = -motors.getSteering();
    if (recoveryDirection == 0) {
        recoveryDirection = random(2) ? 1.0f : -1.0f;
    }
    
    motors.setSpeedPercent(recoverySpeed);
    motors.setSteering(recoveryDirection);
    
    logger.info("Recovery direction: " + String(recoveryDirection) + 
                ", Speed Percent: " + String(recoverySpeed), LogContext::Navigation);
}

void RobotLogic::updateRecoveryManeuver() {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - recoveryStartTime;
    
    switch (recoveryPhase) {
        case RecoveryPhase::Backing:
            if (elapsedTime >= backupDuration) {
                // Keep same steering direction but go forward
                recoveryPhase = RecoveryPhase::Turning;
                recoveryStartTime = currentTime;
                motors.setSpeedPercent(50);  // Half speed for turning
            }
            break;
            
        case RecoveryPhase::Turning:
            if (elapsedTime >= turnDuration) {
                // Recovery complete
                isPerformingRecovery = false;
                motors.stop();
            }
            break;
    }
}

float RobotLogic::calculateSteering(uint16_t left, uint16_t right) {
    // Use exponential decay based on distance
    float leftTurn = exp(-TURN_EXPONENT * left);
    float rightTurn = exp(-TURN_EXPONENT * right);
    return rightTurn - leftTurn;  // Positive = turn right, negative = turn left
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
        50,   // Half speed (50%)
        100   // Full speed (100%)
    );
}
