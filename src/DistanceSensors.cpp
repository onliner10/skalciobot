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
        switch(currentSensor) {
            case LEFT_SENSOR:
                leftSensor.startAsync(SENSOR_READ_TIMEOUT * 1000);
                nextMeasurementTime = now + SENSOR_CYCLE_TIME;  // Wait full 60ms before next sensor
                break;
            case RIGHT_SENSOR:
                rightSensor.startAsync(SENSOR_READ_TIMEOUT * 1000);
                nextMeasurementTime = now + SENSOR_CYCLE_TIME;
                break;
            case FRONT_SENSOR:
                frontSensor.startAsync(SENSOR_READ_TIMEOUT * 1000);
                nextMeasurementTime = now + SENSOR_CYCLE_TIME;
                break;
        }
        measurementStarted = true;
        return;
    }

    bool sensorFinished = false;
    uint16_t distance = 0;

    switch(currentSensor) {
        case LEFT_SENSOR:
            sensorFinished = leftSensor.isFinished();
            if (sensorFinished) distance = leftSensor.getDist_mm();
            break;
        case RIGHT_SENSOR:
            sensorFinished = rightSensor.isFinished();
            if (sensorFinished) distance = rightSensor.getDist_mm();
            break;
        case FRONT_SENSOR:
            sensorFinished = frontSensor.isFinished();
            if (sensorFinished) distance = frontSensor.getDist_mm();
            break;
    }

    if (measurementStarted && sensorFinished) {
        if (distance == 0) {
            const char* sensorNames[] = {"Front", "Left", "Right"};
            logger.debug(String(sensorNames[currentSensor]) + " sensor - No echo", LogContext::Sensor);
            lastMeasurements[currentSensor] = MAX_SENSOR_DISTANCE;
        } else {
            lastMeasurements[currentSensor] = min(distance, (uint16_t)MAX_SENSOR_DISTANCE);
        }
        
        lastReadTime[currentSensor] = now;
        measurementStarted = false;
        measurementUpdated = true;  // Set flag as soon as any sensor is updated
        
        // Move to next sensor with proper casting
        currentSensor = static_cast<SensorIndex>((static_cast<int>(currentSensor) + 1) % NUM_SENSORS);
        
        // Set delay only if we're not done with all sensors
        if (currentSensor != 0) {
            nextMeasurementTime = now + MEASUREMENT_SPACING;
        } else {
            nextMeasurementTime = now;
        }
    }
}
