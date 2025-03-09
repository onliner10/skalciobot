#pragma once
#include "MotorController.h"
#include "DistanceSensors.h"
#include "Logger.h"
#include "config.h"

class RobotLogic {
private:
    static constexpr int HISTORY_SIZE = 40;  // 2000ms / 50ms = 40 readings
    static constexpr float STUCK_THRESHOLD_FACTOR = 0.2f;  // 20% of average movement
    static constexpr float ALPHA = 0.1f;  // EMA factor

    MotorController& motors;
    DistanceSensors& sensors;
    Logger& logger;
    bool active = false;
    unsigned long lastPositionChange = 0;
    const unsigned long STUCK_TIMEOUT = 3000;

    // Circular buffer for recent changes
    float recentChanges[HISTORY_SIZE] = {0};
    int changeIndex = 0;
    float movingAverage = 0;  // Exponential moving average of changes

    unsigned long backupStartTime = 0;
    bool isBackingUp = false;
    bool manualMode = false;

    bool isInitialized = false;
    static const int INIT_READINGS = 5;  // Number of readings needed before stuck detection
    int initReadingCount = 0;
    unsigned long initStartTime = 0;    // When we started measurements
    bool stuckDetectionEnabled = false; // Whether we've collected enough data

    bool isStuck();
    void handleStuckState();
    float calculateAverageChange();
    void updateMovingAverage(float newChange);
    void startBackup();
    bool updateBackup();

public:
    RobotLogic(MotorController& m, DistanceSensors& s, Logger& l) 
        : motors(m), sensors(s), logger(l), active(false), manualMode(false) {}
    
    void begin();
    void update();
    void setActive(bool state);
    bool isActive() { return active; }
    void setManualMode(bool manual);
    bool isManualMode() const { return manualMode; }
};
