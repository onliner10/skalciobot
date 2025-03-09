#include "RobotLogic.h"

void RobotLogic::begin() {
    motors.begin();
    sensors.begin();
}

void RobotLogic::update() {
    if (!active || manualMode) {
        return;
    }

    // Check if stuck (both encoders near zero)
    if (abs(motors.getLeftRpm()) < STUCK_RPM_THRESHOLD && 
        abs(motors.getRightRpm()) < STUCK_RPM_THRESHOLD) {
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
    motors.setRpm(calculateTargetRpm(front));
}

void RobotLogic::handleStuckState() {
    logger.warning("Robot stuck - starting recovery", LogContext::Navigation);
    isPerformingRecovery = true;
    recoveryStartTime = millis();
    backupDuration = random(BACKUP_MIN_TIME, BACKUP_MAX_TIME);
    turnDuration = random(TURN_MIN_TIME, TURN_MAX_TIME);
    recoveryPhase = RecoveryPhase::Backing;
    
    // Start backing up
    motors.setRpm(-MAX_RPM);
    motors.setSteering(0);
}

void RobotLogic::setActive(bool state) {
    logger.info(state ? "Activating autonomous mode" : "Deactivating autonomous mode",
                LogContext::ModeSwitch);
    active = state;
    if (!state) {
        motors.stop();
    }
}

void RobotLogic::setManualMode(bool manual) {
    manualMode = manual;
    if (manual) {
        motors.stop();
    }
}

float RobotLogic::calculateSteering(uint16_t left, uint16_t right) {
    // Use exponential decay based on distance
    // e^(-kx) where k is TURN_EXPONENT and x is distance in mm
    float leftTurn = exp(-TURN_EXPONENT * left);
    float rightTurn = exp(-TURN_EXPONENT * right);
    
    // When distance = MIN_TURN_DISTANCE (150mm), turn factor should be ~1.0
    // When distance = 500mm, turn factor should be ~0.1
    // When distance = 1000mm, turn factor should be ~0.01
    
    return rightTurn - leftTurn;
}

float RobotLogic::calculateFrontMultiplier(uint16_t front) {
    if (front >= MAX_TURN_DISTANCE) return 1.0f;
    
    float normalizedDist = (float)(front - MIN_TURN_DISTANCE) / 
                          (float)(MAX_TURN_DISTANCE - MIN_TURN_DISTANCE);
    normalizedDist = constrain(normalizedDist, 0.0f, 1.0f);
    
    return 1.0f + ((FRONT_MULTIPLIER_MAX - 1.0f) * (1.0f - normalizedDist));
}

double RobotLogic::calculateTargetRpm(uint16_t front) {
    return map(
        constrain(front, MIN_SPEED_DISTANCE, MAX_SPEED_DISTANCE),
        MIN_SPEED_DISTANCE,
        MAX_SPEED_DISTANCE,
        MIN_RPM,
        MAX_RPM
    );
}

void RobotLogic::updateRecoveryManeuver() {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - recoveryStartTime;
    
    switch (recoveryPhase) {
        case RecoveryPhase::Backing:
            if (elapsedTime >= backupDuration) {
                // Switch to turning
                recoveryPhase = RecoveryPhase::Turning;
                recoveryStartTime = currentTime;
                motors.setRpm(MAX_RPM * 0.5);  // Half speed for turning
                motors.setSteering(random(2) ? 1.0f : -1.0f);
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
