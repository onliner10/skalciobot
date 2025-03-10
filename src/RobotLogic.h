#pragma once
#include "MotorController.h"
#include "DistanceSensors.h"
#include "Logger.h"
#include "config.h"

enum class RecoveryPhase {
    Backing,
    Turning
};

class RobotLogic {
private:
    MotorController& motors;
    DistanceSensors& sensors;
    Logger& logger;
    bool isAutonomous = false;

    // Recovery state
    bool isPerformingRecovery = false;
    unsigned long recoveryStartTime = 0;
    unsigned long backupDuration = 0;
    unsigned long turnDuration = 0;
    RecoveryPhase recoveryPhase = RecoveryPhase::Backing;

    float calculateSteering(uint16_t left, uint16_t right);
    float calculateFrontMultiplier(uint16_t front);
    int calculateTargetSpeed(uint16_t front);
    void handleStuckState();
    void updateRecoveryManeuver();

public:
    RobotLogic(MotorController& m, DistanceSensors& s, Logger& l) 
        : motors(m), sensors(s), logger(l), isAutonomous(false) {}
    
    void begin();
    void update();
    bool isAuto() const { return isAutonomous; }
    void setAuto(bool enabled);  // Move implementation to cpp
    bool toggleMode() { 
        setAuto(!isAutonomous);
        return !isAutonomous;  // Return true for MANUAL
    }
};
