#include "OTAManager.h"

OTAManager* OTAManager::instance = nullptr;

OTAManager::OTAManager(Logger& logger, RobotState& robotState)
    : server(new WebServer(80)), logger(logger), robotState(robotState), 
      active(true), progress_millis(0) {
    instance = this;
}

OTAManager::~OTAManager() {
    cleanup();
    instance = nullptr;
}

void OTAManager::cleanup() {
    if (active) {
        server->stop();
        active = false;
        logger.info("OTA resources freed", LogContext::System);
    }
}

void OTAManager::begin() {
    ElegantOTA.begin(server.get(), "");
    ElegantOTA.onStart(handleStart);
    ElegantOTA.onProgress(handleProgress);
    ElegantOTA.onEnd(handleEnd);
    server->begin();
}

void OTAManager::update() {
    if (active && !robotState.isOff()) {
        cleanup();
        return;
    }

    if (active) {
        server->handleClient();
        ElegantOTA.loop();
    }
}

void OTAManager::handleStart() {
    if (instance) instance->onStart();
}

void OTAManager::handleProgress(size_t current, size_t final) {
    if (instance) instance->onProgress(current, final);
}

void OTAManager::handleEnd(bool success) {
    if (instance) instance->onEnd(success);
}

void OTAManager::onStart() {
    logger.info("OTA update started!", LogContext::System);
}

void OTAManager::onProgress(size_t current, size_t final) {
    if (millis() - progress_millis > 1000) {
        progress_millis = millis();
        logger.info("OTA Progress: " + String(current) + " / " + String(final), LogContext::System);
    }
}

void OTAManager::onEnd(bool success) {
    if (success) {
        logger.info("OTA update finished successfully!", LogContext::System);
    } else {
        logger.error("There was an error during OTA update!", LogContext::System);
    }
}
