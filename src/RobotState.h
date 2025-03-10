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

public:
    RobotState(Logger& l) : logger(l) {}
    
    OperationMode getMode() const { return mode; }
    void setMode(OperationMode newMode) {
        if (mode == newMode) return;
        mode = newMode;
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
