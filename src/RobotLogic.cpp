#include "RobotLogic.h"

void RobotLogic::begin() {
    motors.begin();
    sensors.begin();
}

void RobotLogic::update() {
    if (!state.isAuto()) {
        return;  // Only run autonomous logic in Auto mode
    }

    stuckDetector.update();

    // Handle backup maneuver with highest priority
    if (backupUntil > 0) {
        unsigned long currentTime = millis();
        // Currently in backup sequence
        if (currentTime >= backupUntil) {
            logger.info("Backup complete", LogContext::Navigation);
            backupUntil = 0;
            motors.setBackupMode(false);  // Clear backup mode flag
            motors.stop();
        } else {
            // Force backup movement with direct motor control to avoid steering issues
            int backupTime = backupUntil - currentTime;
            logger.info("Backing up: " + String(backupTime) + "ms remaining", LogContext::Navigation);
            
            // Ensure backup mode is set
            motors.setBackupMode(true);
            
            // Bypass normal motor control to ensure straight backup
            // Force direct PWM control instead of using speed + steering
            int pwm = (STUCK_BACKUP_SPEED / 100.0f) * ((1 << MOTOR_PWM_RESOLUTION) - 1);
            motors.getLeftMotor().setPwm(-pwm);  // Negative for backward
            motors.getRightMotor().setPwm(-pwm);  // Negative for backward
            
            return;
        }
    }

    // Check for stuck condition after handling any active backup
    if (stuckDetector.isStuck()) {
        // Start backup sequence
        logger.info("STUCK DETECTED! Starting backup maneuver", LogContext::Navigation);
        
        // Complete stop before changing direction
        motors.stop();
        delay(50); // Short delay to ensure motors stop
        
        // Set backup duration
        backupUntil = millis() + random(STUCK_BACKUP_MIN_TIME, STUCK_BACKUP_MAX_TIME);
        
        // Start backup immediately
        motors.setSteering(0);
        motors.setSpeedPercent(STUCK_BACKUP_SPEED);
        return;
    }

    // Only proceed with normal navigation if not in backup mode
    if (!sensors.hasNewMeasurements()) {
        return;  // Only update when new measurements are available
    }

    // Normal navigation logic
    uint16_t front = sensors.getFrontDistance();
    uint16_t left = sensors.getLeftDistance();
    uint16_t right = sensors.getRightDistance();

    float steering = calculateSteering(left, right, front);
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
        speedFactor = pow(speedFactor, 2);  // Square for more aggressive slowdown
        return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT));
    } else {
        // Quick exponential increase above threshold
        float overshoot = (float)(minDistance - SPEED_THRESHOLD_MM) / (MAX_SENSOR_DISTANCE - SPEED_THRESHOLD_MM);
        float speedFactor = 0.5f + (1.0f - exp(-overshoot * 3)) * 0.5f;
        return MIN_SPEED_PERCENT + (speedFactor * (MAX_SPEED_PERCENT - MIN_SPEED_PERCENT));
    }
}

// Add this method to test backup
void RobotLogic::testBackup() {
    if (!state.isManual()) {
        return;  // Only allow in manual mode
    }
    
    logger.info("Starting backup test", LogContext::Navigation);
    
    // Complete stop before changing direction
    motors.stop();
    delay(50); // Short delay to ensure motors stop
    
    // Set backup mode to prevent interference from motor controller updates
    motors.setBackupMode(true);
    
    // Set backup duration
    backupUntil = millis() + 2000;  // 2 seconds backup
    
    // Start backup immediately - use direct PWM control
    int pwm = (STUCK_BACKUP_SPEED / 100.0f) * ((1 << MOTOR_PWM_RESOLUTION) - 1);
    motors.getLeftMotor().setPwm(-pwm);  // Negative for backward
    motors.getRightMotor().setPwm(-pwm);  // Negative for backward
}
