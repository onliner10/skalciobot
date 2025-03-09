#pragma once
#include <ESPAsyncWebServer.h>
#include "RobotLogic.h"
#include "MotorController.h"
#include "DistanceSensors.h"
#include "Logger.h"
#include "loggers/WebLogger.h"

class WebInterface {
private:
    AsyncWebServer server;
    RobotLogic& robot;
    MotorController& motors;
    DistanceSensors& sensors;
    Logger& logger;
    WebLogger& webLogger;  // Keep this for getting logs

public:
    WebInterface(RobotLogic& r, MotorController& m, DistanceSensors& s, 
                Logger& l, WebLogger& wl)
        : server(80), robot(r), motors(m), sensors(s), 
          logger(l), webLogger(wl) {}  // Move implementation here
    
    void begin();
};
