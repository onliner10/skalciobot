#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "MotorController.h"
#include "DistanceSensors.h"
#include "RobotLogic.h"
#include "WebInterface.h"
#include "credentials.h"
#include <PCF8574.h>
#include "loggers/SerialLogger.h"
#include "loggers/WebLogger.h"
#include "loggers/LogLevelDecorator.h"
#include "loggers/LedLogger.h"

// Create logger chain
LedLogger* ledLogger = new LedLogger(LED_BUILTIN, nullptr);  // Fix null to nullptr
WebLogger* webLogger = new WebLogger(ledLogger);  // Fix null to nullptr
LogLevelDecorator* levelLogger = new LogLevelDecorator(webLogger, LOG_LEVEL, FILTERED_CONTEXTS);

// Create core components with logger
MotorController motors(
    STEERING_MOTOR_1, STEERING_MOTOR_2,
    DRIVE_MOTOR_1, DRIVE_MOTOR_2,
    MOTOR_FLT, MOTOR_SLEEP, MOTOR_FLT  // Pass MOTOR_FLT twice since we only use it
);
PCF8574 pcf(0x20);
DistanceSensors sensors(pcf);  // Remove logger parameter
RobotLogic robot(motors, sensors, *levelLogger);

// Create web interface with both logger and webLogger
WebInterface web(robot, motors, sensors, *levelLogger, *webLogger);

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);  // Set to 100kHz
    
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

    robot.begin();
    web.begin();
    
    levelLogger->info("System boot complete", LogContext::Boot);
}

void loop() {
    sensors.update();  // Keep cycling through sensors
    robot.update();
    levelLogger->update();  // Update logger chain
    
    delay(10);
}