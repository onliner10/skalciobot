#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
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

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create web interface with webLogger
WebInterface web(server, robot, motors, sensors, *webLogger);

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
    
    // Initialize web interface (this will add its routes to the server)
    web.begin();
    
    // Start the server after all routes are set up
    server.begin();
    
    levelLogger->info("Web interface ready", LogContext::Boot);

    robot.begin();
    
    levelLogger->info("System boot complete", LogContext::Boot);
}

void loop() {
    sensors.update();  // Keep cycling through sensors
    robot.update();
    levelLogger->update();  // Update logger chain
    
    delay(10);
}