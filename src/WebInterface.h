#pragma once
#include <ESPAsyncWebServer.h>
#include "RobotLogic.h"
#include "MotorController.h"
#include "DistanceSensors.h"
#include "loggers/WebLogger.h"

class WebInterface {
private:
    AsyncWebServer& server;
    RobotLogic& robot;
    MotorController& motors;
    DistanceSensors& sensors;
    WebLogger& webLogger;

public:
    WebInterface(AsyncWebServer& srv, RobotLogic& r, MotorController& m, DistanceSensors& s, WebLogger& wl)
        : server(srv), robot(r), motors(m), sensors(s), webLogger(wl) {}

    void begin();
};
