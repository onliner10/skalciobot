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
        frontSensor.startAsync(30000);
        leftSensor.startAsync(30000);
        rightSensor.startAsync(30000);
        measurementStarted = true;
        return;
    }

    if (measurementStarted && 
        frontSensor.isFinished() && 
        leftSensor.isFinished() && 
        rightSensor.isFinished()) {
        
        // Store raw readings directly
        lastMeasurements[FRONT_SENSOR] = frontSensor.getDist_mm();
        lastMeasurements[LEFT_SENSOR] = leftSensor.getDist_mm();
        lastMeasurements[RIGHT_SENSOR] = rightSensor.getDist_mm();

        for(int i = 0; i < NUM_SENSORS; i++) {
            lastReadTime[i] = now;
        }

        measurementStarted = false;
        nextMeasurementTime = now + MEASUREMENT_SPACING;
    }
}
