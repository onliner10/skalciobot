#pragma once
#include "Logger.h"

enum class OperationMode {
    Off,
    Manual,
    Auto
};

class RobotState {
private:
    OperationMode mode = OperationMode::Off;
    Logger& logger;
    bool autoMode = false;
    bool sleeping = true;  // Start with motors sleeping
    const int sleepPin;

public:
    RobotState(Logger& l, int sleepPinNumber) 
        : logger(l), sleepPin(sleepPinNumber) {
        pinMode(sleepPin, OUTPUT);
        digitalWrite(sleepPin, sleeping ? LOW : HIGH);
    }

    OperationMode getMode() const { return mode; }
    void setMode(OperationMode newMode) {
        if (mode == newMode) return;
        mode = newMode;
        
        // Control motor sleep based on Off state
        bool shouldSleep = (newMode == OperationMode::Off);
        if (sleeping != shouldSleep) {
            sleeping = shouldSleep;
            digitalWrite(sleepPin, sleeping ? LOW : HIGH);
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
};
