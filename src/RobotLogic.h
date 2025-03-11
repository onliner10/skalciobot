#pragma once
#include "MotorController.h"
#include "DistanceSensors.h"
#include "Logger.h"
#include "RobotState.h"
#include "config.h"

class RobotLogic {
private:
    MotorController& motors;
    DistanceSensors& sensors;
    Logger& logger;
    RobotState& state;

    float calculateSteering(uint16_t left, uint16_t right);
    float calculateFrontMultiplier(uint16_t front);
    int calculateTargetSpeed(uint16_t front);

public:
    RobotLogic(MotorController& m, DistanceSensors& s, Logger& l, RobotState& st)
        : motors(m), sensors(s), logger(l), state(st) {}
    
    void begin();
    void update();
    bool isAuto() const { return state.isAuto(); }
    bool isManual() const { return state.isManual(); }
    bool isOff() const { return state.isOff(); }
    void setState(OperationMode newMode) { state.setMode(newMode); }
};
