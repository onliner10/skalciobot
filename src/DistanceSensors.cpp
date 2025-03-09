#include "DistanceSensors.h"

const uint8_t DistanceSensors::SENSOR_ADDRESSES[3] = {0x30, 0x31, 0x32};

bool DistanceSensors::begin() {
    if (!pcf.begin()) {
        return false;
    }

    // First disable all sensors
    for(int i = 0; i < 3; i++) {
        pcf.write(i, LOW);
    }
    delay(50);  // Give more time to ensure reset

    // Initialize each sensor one by one
    for(int i = 0; i < 3; i++) {
        // Enable current sensor, keeping others disabled
        pcf.write(i, HIGH);
        delay(50);  // Give sensor time to wake up
        
        if(!sensors[i].init()) {
            return false;
        }
        
        sensors[i].setAddress(SENSOR_ADDRESSES[i]);
        sensors[i].startContinuous();
        delay(50);  // Wait for continuous mode to start
    }
    
    return true;
}

void DistanceSensors::update() {
    unsigned long now = millis();
    if (now < nextReadTime) {
        return;
    }

    // Read all sensors at once
    for(int i = 0; i < 3; i++) {
        uint16_t reading = sensors[i].readRangeContinuousMillimeters();
        lastMeasurements[i] = reading;
        lastReadTime[i] = now;
    }
    
    nextReadTime = now + SENSOR_CYCLE_DELAY;
}

uint16_t DistanceSensors::getValidatedDistance(uint16_t reading) const {
    return reading;
}
