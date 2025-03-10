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

// Create shared robot state
RobotState robotState(*levelLogger);

// Create motor instances
Motor leftMotor(LEFT_MOTOR_IN1, LEFT_MOTOR_IN2, ENCODER_LEFT, *levelLogger);
Motor rightMotor(RIGHT_MOTOR_IN1, RIGHT_MOTOR_IN2, ENCODER_RIGHT, *levelLogger);

// Create core components with shared state
MotorController motors(leftMotor, rightMotor, MOTOR_SLEEP, MOTOR_FLT, robotState);
DistanceSensors sensors(*levelLogger);  // Pass logger to sensors
RobotLogic robot(motors, sensors, *levelLogger, robotState);

// Create separate servers for OTA and application
WebServer otaServer(80);        // OTA updates
WebServer appServer(8080);      // Main application

// Create web interface with all dependencies
WebInterface web(appServer, robot, motors, leftMotor, rightMotor, sensors, *webLogger, robotState);

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
    MDNS.addService("http", "tcp", 80);    // Web interface
    MDNS.addService("arduino", "tcp", 3232); // OTA port
    
    // Initialize ElegantOTA
    ElegantOTA.begin(&otaServer, "");  // Add empty auth
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    otaServer.begin();
    
    // Initialize application server on port 8080
    web.begin();
    appServer.begin();
    
    levelLogger->info("Web interfaces ready", LogContext::Boot);

    robot.begin();
    
    levelLogger->info("System boot complete", LogContext::Boot);
}

void loop() {
    sensors.update();  // Keep cycling through sensors
    leftMotor.update();  // Add motor updates
    rightMotor.update();
    robot.update();
    levelLogger->update();  // Update logger chain
    
    otaServer.handleClient();  // Handle OTA requests
    appServer.handleClient();  // Handle application requests
    ElegantOTA.loop();  // Handle OTA updates
    
    delay(10);
}