#include "RobotLogic.h"

void RobotLogic::begin() {
    motors.begin();
    sensors.begin();
    initStartTime = millis();
}

void RobotLogic::update() {
    if (!active || manualMode) {
        return;
    }

    if (isStuck()) {
        logger.warning("Robot detected stuck state", LogContext::Navigation);
        handleStuckState();
        return;
    }

    if (isBackingUp) {
        logger.info("Robot is backing up", LogContext::Navigation);
        if (!updateBackup()) {
            return;
        }
    }

    uint16_t front = sensors.getFrontDistance();
    uint16_t left = (sensors.getLeftDistance() / DISTANCE_ROUNDING) * DISTANCE_ROUNDING;
    uint16_t right = (sensors.getRightDistance() / DISTANCE_ROUNDING) * DISTANCE_ROUNDING;

    logger.debug("Distances - Front: " + String(front) + 
               "mm, Left: " + String(left) + 
               "mm, Right: " + String(right) + "mm",
               LogContext::Sensor);

    if (front < FRONT_OBSTACLE_THRESHOLD) {
        logger.info("Front obstacle detected, backing up", LogContext::Navigation);
        startBackup();
    } else {
        int speed = map(
            constrain(front, MIN_SPEED_DISTANCE, MAX_SPEED_DISTANCE),
            MIN_SPEED_DISTANCE,
            MAX_SPEED_DISTANCE,
            MIN_FORWARD_SPEED,
            MAX_FORWARD_SPEED
        );
        
        logger.debug("Setting speed to: " + String(speed), LogContext::Motor);
        motors.setSpeed(speed);
        
        int difference = abs(left - right);
        logger.debug("Left-right difference: " + String(difference), LogContext::Navigation);
        
        if (difference <= CLEARANCE_TOLERANCE) {
            logger.info("Going straight", LogContext::Navigation);
            motors.setSteering(STRAIGHT);
        } else if (left > right) {
            logger.info("Turning left", LogContext::Navigation);
            motors.setSteering(LEFT);
        } else {
            logger.debug("Turning right", LogContext::Navigation);
            motors.setSteering(RIGHT);
        }
    }
}

bool RobotLogic::isStuck() {
    static uint16_t lastFront = 0;
    static uint16_t lastLeft = 0;
    static uint16_t lastRight = 0;
    
    uint16_t front = sensors.getFrontDistance();
    uint16_t left = sensors.getLeftDistance();
    uint16_t right = sensors.getRightDistance();

    if (!stuckDetectionEnabled) {
        if (millis() - initStartTime < STUCK_INIT_TIME) {
            lastFront = front;
            lastLeft = left;
            lastRight = right;
            return false;
        }
        stuckDetectionEnabled = true;
    }

    if (!isInitialized) {
        lastFront = front;
        lastLeft = left;
        lastRight = right;
        initReadingCount++;
        
        if (initReadingCount >= INIT_READINGS) {
            isInitialized = true;
        }
        return false;
    }
    
    // Calculate total change in sensor readings
    float totalChange = abs(front - lastFront) + 
                       abs(left - lastLeft) + 
                       abs(right - lastRight);

    // Update moving averages
    updateMovingAverage(totalChange);
    
    // Store in circular buffer
    recentChanges[changeIndex] = totalChange;
    changeIndex = (changeIndex + 1) % HISTORY_SIZE;
    
    // Calculate recent average change
    float recentAverage = calculateAverageChange();
    
    // We're stuck if recent changes are significantly below the long-term average
    bool isCurrentlyStuck = recentAverage < (movingAverage * STUCK_THRESHOLD_FACTOR);
    
    if (isCurrentlyStuck) {
        if (millis() - lastPositionChange > STUCK_TIMEOUT) {
            return true;
        }
    } else {
        lastPositionChange = millis();
    }
    
    lastFront = front;
    lastLeft = left;
    lastRight = right;
    
    return false;
}

float RobotLogic::calculateAverageChange() {
    float sum = 0;
    int count = 0;
    
    // Calculate average of recent changes
    for(int i = 0; i < HISTORY_SIZE; i++) {
        if(recentChanges[i] > 0) {  // Only count initialized values
            sum += recentChanges[i];
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0;
}

void RobotLogic::updateMovingAverage(float newChange) {
    if(movingAverage == 0) {
        movingAverage = newChange;  // Initialize on first reading
    } else {
        // Exponential Moving Average
        movingAverage = (ALPHA * newChange) + ((1.0 - ALPHA) * movingAverage);
    }
}

void RobotLogic::handleStuckState() {
    logger.warning("Robot detected stuck state", LogContext::Navigation);
    startBackup();  
    delay(BACKUP_DURATION);  
    lastPositionChange = millis();
}

void RobotLogic::setActive(bool state) {
    logger.info(state ? "Activating autonomous mode" : "Deactivating autonomous mode",
                LogContext::ModeSwitch);
    active = state;
    if (!state) {
        motors.stop();
    } else {
        // Reset all stuck detection state
        stuckDetectionEnabled = false;
        initStartTime = millis();
        isInitialized = false;
        initReadingCount = 0;
        movingAverage = 0;
        for(int i = 0; i < HISTORY_SIZE; i++) {
            recentChanges[i] = 0;
        }
        // Make sure motors are ready to move
        motors.setSpeed(100);
        motors.setSteering(STRAIGHT);
    }
}

void RobotLogic::startBackup() {
    logger.info("Starting backup maneuver", LogContext::Navigation);
    isBackingUp = true;
    backupStartTime = millis();
    motors.setSpeed(-100);  // Use full speed for backup
    motors.setSteering(random(2) ? RIGHT : LEFT);
}

bool RobotLogic::updateBackup() {
    if (millis() - backupStartTime >= BACKUP_DURATION) {
        isBackingUp = false;
        return true;  // Backup completed
    }
    return false;  // Still backing up
}

void RobotLogic::setManualMode(bool manual) {
    manualMode = manual;
    if (manual) {
        motors.stop();  // Stop all motors when entering manual mode
    }
}
