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
    bool active = false;
    bool manualMode = false;

    // Recovery state
    bool isPerformingRecovery = false;
    unsigned long recoveryStartTime = 0;
    unsigned long backupDuration = 0;
    unsigned long turnDuration = 0;
    RecoveryPhase recoveryPhase = RecoveryPhase::Backing;

    float calculateSteering(uint16_t left, uint16_t right);
    float calculateFrontMultiplier(uint16_t front);
    double calculateTargetRpm(uint16_t front);
    void handleStuckState();
    void updateRecoveryManeuver();

public:
    RobotLogic(MotorController& m, DistanceSensors& s, Logger& l) 
        : motors(m), sensors(s), logger(l), active(false), manualMode(false) {}
    
    void begin();
    void update();
    void setActive(bool state);
    bool isActive() { return active; }
    void setManualMode(bool manual);
    bool isManualMode() const { return manualMode; }
    void toggle() { setActive(!active); }
    bool toggleMode() { 
        setManualMode(!manualMode); 
        return manualMode;
    }
};
