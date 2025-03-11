#include "DistanceSensors.h"

bool DistanceSensors::begin() {
    bool success = true;
    success &= frontSensor.beginAsync();
    success &= leftSensor.beginAsync();
    success &= rightSensor.beginAsync();
    
    if (!success) {
        logger.error("Failed to initialize distance sensors", LogContext::Sensor);
    }
    return success;
}

void DistanceSensors::update() {
    unsigned long now = millis();
    
    if (!measurementStarted && now >= nextMeasurementTime) {
        frontSensor.startAsync(MEASUREMENT_SPACING * 1000);
        leftSensor.startAsync(MEASUREMENT_SPACING * 1000);
        rightSensor.startAsync(MEASUREMENT_SPACING * 1000);
        measurementStarted = true;
        return;
    }

    if (measurementStarted && 
        frontSensor.isFinished() && 
        leftSensor.isFinished() && 
        rightSensor.isFinished()) {
        
        // Get readings and handle errors
        uint16_t frontDist = frontSensor.getDist_mm();
        uint16_t leftDist = leftSensor.getDist_mm();
        uint16_t rightDist = rightSensor.getDist_mm();
        
        // Store readings with error handling
        lastMeasurements[FRONT_SENSOR] = frontDist > 0 ? frontDist : MAX_SENSOR_DISTANCE;
        lastMeasurements[LEFT_SENSOR] = leftDist > 0 ? leftDist : MAX_SENSOR_DISTANCE;
        lastMeasurements[RIGHT_SENSOR] = rightDist > 0 ? rightDist : MAX_SENSOR_DISTANCE;

        // Log errors if any
        if (frontDist == 0) logger.debug("Front sensor - No echo", LogContext::Sensor);
        if (leftDist == 0) logger.debug("Left sensor - No echo", LogContext::Sensor);
        if (rightDist == 0) logger.debug("Right sensor - No echo", LogContext::Sensor);

        for(int i = 0; i < NUM_SENSORS; i++) {
            lastReadTime[i] = now;
        }

        measurementStarted = false;
        measurementUpdated = true;  // Set flag for new measurements
        nextMeasurementTime = now + MEASUREMENT_SPACING;
    }
}
