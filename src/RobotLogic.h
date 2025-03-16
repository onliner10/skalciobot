#pragma once
#include "MotorController.h"
#include "DistanceSensors.h"
#include "Logger.h"
#include "RobotState.h"
#include "config.h"
#include "StuckDetector.h"

class RobotLogic {
private:
    MotorController& motors;
    DistanceSensors& sensors;
    Logger& logger;
    RobotState& state;
    StuckDetector stuckDetector;
    unsigned long backupUntil = 0;  // Timestamp when to stop backing up

    float calculateSteering(uint16_t left, uint16_t right, uint16_t front);
    float calculateFrontMultiplier(uint16_t front);
    int calculateTargetSpeed(uint16_t front);

public:
    RobotLogic(MotorController& m, DistanceSensors& s, Logger& l, RobotState& st)
        : motors(m), sensors(s), logger(l), state(st), 
          stuckDetector(m.getLeftMotor(), m.getRightMotor(), s) {}
    
    void begin();
    void update();
    bool isAuto() const { return state.isAuto(); }
    bool isManual() const { return state.isManual(); }
    bool isOff() const { return state.isOff(); }
    void setState(OperationMode newMode) { state.setMode(newMode); }
    bool isStuck() const { return stuckDetector.isStuck(); }
    int getBackupTimeRemaining() const { 
        return backupUntil > 0 ? backupUntil - millis() : 0;
    }
    void testBackup();  // Add test function for backup
    void resetStuckDetection() { stuckDetector.resetDetection(); }
};
