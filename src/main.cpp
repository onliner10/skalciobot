#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include "config.h"
#include "MotorController.h"
#include "DistanceSensors.h"
#include "RobotLogic.h"
#include "WebInterface.h"
#include "credentials.h"
#include "loggers/SerialLogger.h"
#include "loggers/WebLogger.h"
#include "loggers/LogLevelDecorator.h"
#include "loggers/LedLogger.h"

// Create logger chain
LedLogger* ledLogger = new LedLogger(LED_BUILTIN, nullptr);  // Fix null to nullptr
WebLogger* webLogger = new WebLogger(ledLogger);  // Fix null to nullptr
LogLevelDecorator* levelLogger = new LogLevelDecorator(webLogger, LOG_LEVEL, FILTERED_CONTEXTS);

// Create motor instances
Motor leftMotor(LEFT_MOTOR_IN1, LEFT_MOTOR_IN2, ENCODER_LEFT);
Motor rightMotor(RIGHT_MOTOR_IN1, RIGHT_MOTOR_IN2, ENCODER_RIGHT);

// Create core components with logger
MotorController motors(leftMotor, rightMotor, MOTOR_SLEEP, MOTOR_FLT);
DistanceSensors sensors;
RobotLogic robot(motors, sensors, *levelLogger);

// Create single WebServer for both OTA and application
WebServer server(80);

// Create web interface with webLogger
WebInterface web(server, robot, motors, sensors, *webLogger);

unsigned long ota_progress_millis = 0;

void onOTAStart() {
    // Log when OTA has started
    Serial.println("OTA update started!");
    levelLogger->info("OTA update started!", LogContext::System);
}

void onOTAProgress(size_t current, size_t final) {
    // Log every 1 second
    if (millis() - ota_progress_millis > 1000) {
        ota_progress_millis = millis();
        Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
        levelLogger->info("OTA Progress: " + String(current) + " / " + String(final), LogContext::System);
    }
}

void onOTAEnd(bool success) {
    // Log when OTA has finished
    if (success) {
        Serial.println("OTA update finished successfully!");
        levelLogger->info("OTA update finished successfully!", LogContext::System);
    } else {
        Serial.println("There was an error during OTA update!");
        levelLogger->error("There was an error during OTA update!", LogContext::System);
    }
}

void setup() {
    Serial.begin(115200);
    
    Serial.println("\nConnecting to WiFi");
    levelLogger->info("Connecting to WiFi", LogContext::Wifi);  // Fix log call
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    String connMsg = "\nWiFi connected! IP: " + WiFi.localIP().toString();
    Serial.println(connMsg);
    levelLogger->info(connMsg, LogContext::Wifi);  // Fix log call

    // Set up mDNS responder
    if(!MDNS.begin("skalciobot")) {
        levelLogger->error("Error setting up mDNS responder!", LogContext::Boot);
    }

    // Initialize ElegantOTA and web interface
    ElegantOTA.begin(&server);
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    
    web.begin();
    server.begin();
    
    levelLogger->info("Web interface ready", LogContext::Boot);

    robot.begin();
    
    levelLogger->info("System boot complete", LogContext::Boot);
}

void loop() {
    sensors.update();  // Keep cycling through sensors
    robot.update();
    levelLogger->update();  // Update logger chain
    
    server.handleClient();  // Handle both OTA and application requests
    ElegantOTA.loop();  // Handle OTA updates
    
    delay(10);
}