#pragma once
#include <Arduino.h>
#include "Motor.h"
#include "DistanceSensors.h"
#include "config.h"

class StuckDetector {
private:
    Motor& leftMotor;
    Motor& rightMotor;
    DistanceSensors& sensors;
    
    uint16_t previousReadings[3] = {0};  // Last readings [front,left,right]
    int16_t deltaHistory[STUCK_HISTORY_SIZE][3] = {0};  // Changes between consecutive readings
    size_t historyIndex = 0;
    unsigned long lastUpdate = 0;
    
    float calculateDeltaStdDev(size_t sensorIndex) const;
    bool isEncoderStuck() const;
    bool isDistanceStuck() const;

public:
    StuckDetector(Motor& left, Motor& right, DistanceSensors& sens)
        : leftMotor(left), rightMotor(right), sensors(sens) {}
        
    void update();
    bool isStuck() const;
};
