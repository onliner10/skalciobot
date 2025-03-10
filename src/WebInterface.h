#pragma once
#include <WebServer.h>
#include "RobotLogic.h"
#include "MotorController.h"
#include "DistanceSensors.h"
#include "loggers/WebLogger.h"

class WebInterface {
private:
    WebServer& server;
    RobotLogic& robot;
    MotorController& motors;
    Motor& leftMotor;
    Motor& rightMotor;
    DistanceSensors& sensors;
    WebLogger& webLogger;

public:
    WebInterface(WebServer& srv, RobotLogic& r, MotorController& m, 
                Motor& left, Motor& right, DistanceSensors& s, WebLogger& wl)
        : server(srv), robot(r), motors(m), 
          leftMotor(left), rightMotor(right), sensors(s), webLogger(wl) {}

    void begin();
    void handle() { server.handleClient(); }
};
