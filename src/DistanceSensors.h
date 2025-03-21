#pragma once
#include <Arduino.h>
#include "HC_SR04.h"
#include "config.h"
#include "Logger.h"

enum SensorIndex {
    LEFT_SENSOR = 0,
    RIGHT_SENSOR = 1,
    FRONT_SENSOR = 2
};

class DistanceSensors {
private:
    Logger& logger;
    uint16_t lastMeasurements[NUM_SENSORS];
    unsigned long lastReadTime[NUM_SENSORS];
    SensorIndex currentSensor = LEFT_SENSOR;  // Changed from uint8_t to SensorIndex
    
    HC_SR04<FRONT_ECHO_PIN> frontSensor;
    HC_SR04<LEFT_ECHO_PIN> leftSensor;
    HC_SR04<RIGHT_ECHO_PIN> rightSensor;
    
    bool measurementStarted = false;
    unsigned long nextMeasurementTime = 0;
    static constexpr unsigned long MEASUREMENT_SPACING = SENSOR_CYCLE_TIME;  // ms between measurements
    bool measurementUpdated = false;

public:
    DistanceSensors(Logger& l) 
        : logger(l),
          frontSensor(FRONT_TRIG_PIN),
          leftSensor(LEFT_TRIG_PIN),
          rightSensor(RIGHT_TRIG_PIN) {
        
        for(int i = 0; i < NUM_SENSORS; i++) {
            lastMeasurements[i] = 0;
            lastReadTime[i] = 0;
        }
    }
    
    // Remove destructor as we no longer have dynamic allocation

    bool begin();
    void update();
    
    uint16_t getFrontDistance() const { return lastMeasurements[FRONT_SENSOR]; }
    uint16_t getLeftDistance() const { return lastMeasurements[LEFT_SENSOR]; }
    uint16_t getRightDistance() const { return lastMeasurements[RIGHT_SENSOR]; }
    unsigned long getLastReadTime(int sensor) const { return lastReadTime[sensor]; }
    bool hasNewMeasurements() const { return measurementUpdated; }
    void clearNewMeasurementsFlag() { measurementUpdated = false; }
};
