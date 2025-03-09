#include "WebInterface.h"

void WebInterface::begin() {
    // Serve root page
    server.on("/", HTTP_GET, [this]() {
        const char* html = "<html><body>"
            "<h1>Robot Control</h1>"
            "<p><a href='/toggle'>Toggle Robot</a></p>"
            "<p><a href='/motors/test'>Test Motors</a></p>"
            "<p><a href='/motors/stop'>Stop Motors</a></p>"
            "<p><a href='/log'>View Logs</a></p>"
            "</body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/status", HTTP_GET, [this]() {
        server.send(200, "text/plain", robot.isActive() ? "ON" : "OFF");
    });

    server.on("/toggle", HTTP_GET, [this]() {
        robot.toggle();
        server.send(200, "text/plain", robot.isActive() ? "ON" : "OFF");
    });

    server.on("/motors/test", HTTP_GET, [this]() {
        motors.test();
        server.send(200, "text/plain", "Running test sequence");
    });

    server.on("/distance", HTTP_GET, [this]() {
        String json = "{";
        json += "\"front\":" + String(sensors.getFrontDistance()) + ",";
        json += "\"left\":" + String(sensors.getLeftDistance()) + ",";
        json += "\"right\":" + String(sensors.getRightDistance());
        json += "}";
        server.send(200, "application/json", json);
    });

    // For parameters, use URI patterns with WebServer
    server.on("/motors/steering", HTTP_GET, [this]() {
        if(server.args() > 0) {
            int steering = server.arg(0).toInt();
            if (steering >= -100 && steering <= 100) {
                motors.setSteering(steering);
                server.send(200, "text/plain", "Steering set to: " + String(steering));
            } else {
                server.send(400, "text/plain", "Invalid steering value");
            }
        }
    });

    server.on("/motors/rpm", HTTP_GET, [this]() {
        if(server.args() > 0) {
            int rpm = server.arg(0).toInt();
            if (rpm >= -255 && rpm <= 255) {
                motors.setRpm(rpm);  // Changed from setRPM
                server.send(200, "text/plain", "RPM set to: " + String(rpm));
            } else {
                server.send(400, "text/plain", "Invalid RPM value");
            }
        }
    });

    server.on("/motors/stop", HTTP_GET, [this]() {
        motors.stop();
        server.send(200, "text/plain", "Motors stopped");
    });

    server.on("/motors/status", HTTP_GET, [this]() {
        String status = "Steering: " + String(motors.getSteering());
        status += ", RPM: " + String(motors.getRpm());  // Changed from getRPM
        server.send(200, "text/plain", status);
    });

    server.on("/mode/toggle", HTTP_GET, [this]() {
        bool newMode = robot.toggleMode();
        server.send(200, "text/plain", newMode ? "MANUAL" : "AUTO");
    });

    server.on("/log", HTTP_GET, [this]() {
        server.send(200, "text/plain", webLogger.getLogs());
    });
}
