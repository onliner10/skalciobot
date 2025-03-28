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
#include "OTAManager.h"

// Create logger chain
LedLogger* ledLogger = new LedLogger(LED_BUILTIN, nullptr);  // Fix null to nullptr
WebLogger* webLogger = new WebLogger(ledLogger);  // Fix null to nullptr
LogLevelDecorator* levelLogger = new LogLevelDecorator(webLogger, LOG_LEVEL, FILTERED_CONTEXTS);

// Create shared robot state
RobotState robotState(*levelLogger, MOTOR_SLEEP);

// Create motors
Motor leftMotor(LEFT_MOTOR_IN1, LEFT_MOTOR_IN2, ENCODER_LEFT, *levelLogger);
Motor rightMotor(RIGHT_MOTOR_IN1, RIGHT_MOTOR_IN2, ENCODER_RIGHT, *levelLogger);

// Create motor controller with logger
MotorController motors(leftMotor, rightMotor, MOTOR_FLT, robotState, *levelLogger);

// Create core components with shared state
DistanceSensors sensors(*levelLogger);  // Pass logger to sensors
RobotLogic robot(motors, sensors, *levelLogger, robotState);

WebServer appServer(8080);      // Main application

// Create web interface with all dependencies
WebInterface web(appServer, robot, motors, leftMotor, rightMotor, sensors, *webLogger, robotState);

// Create OTA manager
OTAManager ota(*levelLogger, robotState);

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
    
    // Initialize OTA
    ota.begin();
    
    // Initialize application server
    web.begin();
    appServer.begin();
    
    levelLogger->info("Web interfaces ready", LogContext::Boot);
    robot.begin();
    levelLogger->info("System boot complete", LogContext::Boot);
    
    // Remove timer initialization from here
}

void loop() {
    sensors.update();
    motors.update();
    robot.update();
    levelLogger->update();
    
    // Check if we should auto-switch to auto mode
    if (robotState.shouldSwitchToAuto()) {
        levelLogger->info("Auto-switching to AUTO mode after inactivity", LogContext::System);
        robotState.setMode(OperationMode::Auto);
    }
    
    ota.update();
    appServer.handleClient();
}