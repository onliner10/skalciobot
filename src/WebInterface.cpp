#include "WebInterface.h"

static String uri_regex(const String& pattern) {
    return "^\\/motors\\/" + pattern + "$";  // Fixed regex pattern
}

void WebInterface::begin() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = R"rawliteral(
            <html><body style='font-family: Arial, sans-serif;'>
            <h1>Robot Control</h1>
            <div style='margin: 20px 0;'>
                Status: <span id='statusText'>Checking...</span>
                <button id='toggleBtn' onclick='toggleRobot()' style='margin-left: 10px;'>
                    Loading...
                </button>
            </div>
            <div style='margin: 20px 0;'>
                <h3>Manual Control</h3>
                <div style='margin: 10px 0;'>
                    <h4>Steering Control</h4>
                    <input type='range' min='-100' max='100' value='0' 
                           oninput='setSteeringValue(this.value)' style='width: 300px'>
                    <span id='steeringValue'>0</span>
                </div>
                <div style='margin: 10px 0;'>
                    <h4>Speed Control</h4>
                    <input type='range' min='-200' max='200' value='0' 
                           oninput='setRpmValue(this.value)' style='width: 300px'>
                    <span id='rpmValue'>0 RPM</span>
                </div>
            </div>
            <div id='motorStatus' style='margin: 10px 0; font-family: monospace;'>
                Motor Status: Waiting...
            </div>
            <div id='sensorData' style='margin: 10px 0; font-family: monospace;'>
                Loading sensor data...
            </div>
            <div id='logArea' style='font-family: monospace; 
                                   white-space: pre; 
                                   background-color: #f0f0f0; 
                                   padding: 10px; 
                                   height: 400px; 
                                   overflow-y: scroll;
                                   font-size: 12px;
                                   line-height: 1.2;'>
            </div>
            <script>
                function updateStatus() { 
                    fetch('/status')
                        .then(response => response.text())
                        .then(data => {
                            // ...existing updateStatus code...
                        });
                }
                // ...rest of existing JavaScript...
            </script>
            </body></html>
        )rawliteral";
        request->send(200, "text/html", html);
    });

    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", robot.isActive() ? "ON" : "OFF");
    });

    server.on("/toggle", HTTP_GET, [this](AsyncWebServerRequest *request){
        bool newState = !robot.isActive();
        robot.setActive(newState);
        request->send(200, "text/plain", robot.isActive() ? "ON" : "OFF");
    });

    server.on("/motors/test", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Simplified test response - actual test is handled by client-side JavaScript
        request->send(200, "text/plain", "Running test sequence");
    });

    server.on("/distance", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sensors.update();  // Ensure sensors are updated before reading
        String json = "{";
        json += "\"front\":" + String(sensors.getFrontDistance()) + ",";
        json += "\"left\":" + String(sensors.getLeftDistance()) + ",";
        json += "\"right\":" + String(sensors.getRightDistance());
        json += "}";
        request->send(200, "application/json", json);
    });

    // Update motor control endpoints with fixed regex patterns
    server.on(uri_regex("steering/(-?\\d+\\.?\\d*)").c_str(), HTTP_GET, 
        [this](AsyncWebServerRequest *request) {
        String value = request->pathArg(0);
        if (value.length() > 0) {
            float steering = value.toFloat();
            motors.setSteering(steering);
            request->send(200, "text/plain", "Steering set to: " + String(steering));
        } else {
            request->send(400, "text/plain", "Invalid steering value");
        }
    });

    server.on(uri_regex("rpm/(-?\\d+)").c_str(), HTTP_GET, 
        [this](AsyncWebServerRequest *request) {
        String value = request->pathArg(0);
        if (value.length() > 0) {
            int rpm = value.toInt();
            motors.setRpm(rpm);
            request->send(200, "text/plain", "RPM set to: " + String(rpm));
        } else {
            request->send(400, "text/plain", "Invalid RPM value");
        }
    });

    server.on("/motors/stop", HTTP_GET, [this](AsyncWebServerRequest *request) {
        motors.stop();
        request->send(200, "text/plain", "Motors stopped");
    });

    server.on("/motors/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String status = "Motor Status:\n";
        status += "Current RPM: " + String(motors.getRpm()) + "\n";
        status += "Steering: " + String(motors.getSteering()) + "\n";
        status += "Fault Status: " + String(motors.checkFault() ? "FAULT DETECTED" : "OK");
        request->send(200, "text/plain", status);
    });

    server.on("/mode/toggle", HTTP_GET, [this](AsyncWebServerRequest *request){
        bool newMode = !robot.isManualMode();
        robot.setManualMode(newMode);
        request->send(200, "text/plain", newMode ? "MANUAL" : "AUTO");
    });

    server.on("/log", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", webLogger.getLogs());
    });

    server.begin();
}
