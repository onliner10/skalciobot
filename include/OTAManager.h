#pragma once

#include <WebServer.h>
#include <ElegantOTA.h>
#include "RobotState.h"
#include "Logger.h"

class OTAManager {
public:
    OTAManager(Logger& logger, RobotState& robotState);
    ~OTAManager();
    void begin();
    void update();
    bool isActive() const { return active; }

private:
    std::unique_ptr<WebServer> server;
    Logger& logger;
    RobotState& robotState;
    bool active;
    unsigned long progress_millis;

    void cleanup();
    void onStart();
    void onProgress(size_t current, size_t final);
    void onEnd(bool success);
    
    static void handleStart();
    static void handleProgress(size_t current, size_t final);
    static void handleEnd(bool success);
    
    static OTAManager* instance;
};
