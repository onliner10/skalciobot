#pragma once
#include <Arduino.h>
#include "HC_SR04.h"
#include "config.h"

class DistanceSensors {
private:
    uint16_t lastMeasurements[NUM_SENSORS];
    unsigned long lastReadTime[NUM_SENSORS];
    
    HC_SR04_BASE* slaves[2];
    HC_SR04<FRONT_ECHO_PIN> sonicMaster;

    bool measurementStarted = false;
    unsigned long nextMeasurementTime = 0;
    static constexpr unsigned long MEASUREMENT_SPACING = 50;  // ms between measurements

    uint16_t getValidatedDistance(uint16_t reading) const;

public:
    DistanceSensors() : sonicMaster(FRONT_TRIG_PIN) {
        slaves[0] = new HC_SR04<LEFT_ECHO_PIN>(LEFT_TRIG_PIN);
        slaves[1] = new HC_SR04<RIGHT_ECHO_PIN>(RIGHT_TRIG_PIN);
        
        for(int i = 0; i < NUM_SENSORS; i++) {
            lastMeasurements[i] = 0;
            lastReadTime[i] = 0;
        }
    }
    
    ~DistanceSensors() {
        delete slaves[0];
        delete slaves[1];
    }

    bool begin();
    void update();
    
    uint16_t getFrontDistance() const { return lastMeasurements[0]; }
    uint16_t getLeftDistance() const { return lastMeasurements[1]; }
    uint16_t getRightDistance() const { return lastMeasurements[2]; }
    unsigned long getLastReadTime(int sensor) const { return lastReadTime[sensor]; }
};
