#include "DistanceSensors.h"

bool DistanceSensors::begin() {
    return sonicMaster.beginAsync();
}

void DistanceSensors::update() {
    unsigned long now = millis();
    
    if (!measurementStarted && now >= nextMeasurementTime) {
        sonicMaster.startAsync(30000);  // 30ms timeout
        measurementStarted = true;
        return;
    }

    if (measurementStarted && sonicMaster.isFinished()) {
        // Read all sensors
        for (int i = 0; i < NUM_SENSORS; i++) {
            uint16_t distance = sonicMaster.getDist_mm(i);
            lastMeasurements[i] = getValidatedDistance(distance);
            lastReadTime[i] = now;
        }

        // Schedule next measurement
        measurementStarted = false;
        nextMeasurementTime = now + MEASUREMENT_SPACING;
    }
}

uint16_t DistanceSensors::getValidatedDistance(uint16_t reading) const {
    if (reading == 0 || reading > 4000) { // HC-SR04 has max range of ~4m
        return SENSOR_INVALID_READING;
    }
    return reading;
}
