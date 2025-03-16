#pragma once
#include "Logger.h"
#include "config.h"  // Add include for AUTO_SWITCH_TIMEOUT

enum class OperationMode {
    Off,
    Manual,
    Auto
};

class RobotState {
private:
    OperationMode mode = OperationMode::Off;
    Logger& logger;
    bool sleeping = true;  // Start with motors sleeping
    const int motorSleepPin;  // Fix name to match constructor
    unsigned long lastActivityTime = 0;
    bool autoSwitchEnabled = true;

public:
    RobotState(Logger& l, int sleepPin) 
        : logger(l), motorSleepPin(sleepPin), mode(OperationMode::Off) {
        pinMode(motorSleepPin, OUTPUT);
        digitalWrite(motorSleepPin, sleeping ? LOW : HIGH);
        lastActivityTime = millis();  // Initialize activity timer
    }

    OperationMode getMode() const { return mode; }
    void setMode(OperationMode newMode) {
        if (mode == newMode) return;
        mode = newMode;
        
        // Control motor sleep based on Off state
        bool shouldSleep = (newMode == OperationMode::Off);
        if (sleeping != shouldSleep) {
            sleeping = shouldSleep;
            digitalWrite(motorSleepPin, sleeping ? LOW : HIGH);
            logger.debug(String("Motors ") + (sleeping ? "sleeping" : "waking up"), 
                        LogContext::Motor);
        }
        
        logger.info("Mode: " + String(
            mode == OperationMode::Off ? "OFF" : 
            mode == OperationMode::Manual ? "MANUAL" : "AUTO"
        ), LogContext::ModeSwitch);
    }
    
    bool isAuto() const { return mode == OperationMode::Auto; }
    bool isManual() const { return mode == OperationMode::Manual; }
    bool isOff() const { return mode == OperationMode::Off; }
    bool isEnabled() const { return !isOff(); }
    void resetActivityTimer() { lastActivityTime = millis(); }
    bool shouldSwitchToAuto() const {
        return autoSwitchEnabled && 
               !isAuto() && 
               (millis() - lastActivityTime > AUTO_SWITCH_TIMEOUT);
    }
    void setAutoSwitchEnabled(bool enabled) { autoSwitchEnabled = enabled; }
    bool isAutoSwitchEnabled() const { return autoSwitchEnabled; }
};
