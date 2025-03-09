#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <PCF8574.h>
#include "config.h"  // Add this include

class DistanceSensors {
private:
    PCF8574& pcf;
    VL53L0X sensors[3];
    uint16_t lastMeasurements[3];
    unsigned long lastReadTime[3];
    static const uint8_t SENSOR_ADDRESSES[3];
    
    int currentSensor = 0;
    unsigned long nextReadTime = 0;
    static const unsigned long SENSOR_CYCLE_DELAY = 40;  // 40ms between readings

    uint16_t getValidatedDistance(uint16_t reading) const;

public:
    DistanceSensors(PCF8574& pcfDevice) 
        : pcf(pcfDevice) {}
        
    bool begin();
    void read() {}  // Just return cached values
    void update();  // Background sensor cycling
    uint16_t getFrontDistance() { return getValidatedDistance(lastMeasurements[0]); }
    uint16_t getLeftDistance() { return getValidatedDistance(lastMeasurements[2]); }
    uint16_t getRightDistance() { return getValidatedDistance(lastMeasurements[1]); }
};
