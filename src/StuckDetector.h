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
    unsigned long lastBackupTime = 0;  // When the last backup completed
    bool isInitialized = false;        // If detector has enough samples to work properly
    
    float calculateDeltaStdDev(size_t sensorIndex) const;
    bool isEncoderStuck() const;
    bool isDistanceStuck() const;
    
    float getSpeedDependentThreshold() const;

public:
    StuckDetector(Motor& left, Motor& right, DistanceSensors& sens)
        : leftMotor(left), rightMotor(right), sensors(sens) {}
        
    void update();
    bool isStuck() const;
    void notifyBackupCompleted() { lastBackupTime = millis(); }
    void resetDetection() { 
        isInitialized = false; 
        historyIndex = 0;
        lastBackupTime = millis();
    }
};
