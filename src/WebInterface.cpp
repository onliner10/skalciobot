#include "WebInterface.h"

void WebInterface::begin() {
    // Serve root page with auto-escaped HTML template
    server.on("/", HTTP_GET, [this]() {
        const char* html = R"rawliteral(
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; }
        .sensor-value { font-weight: bold; }
        .control-group { margin: 20px 0; }
        .slider-container { width: 100%; max-width: 400px; }
        #logs { height: 200px; overflow-y: scroll; border: 1px solid #ccc; padding: 10px; font-family: monospace; margin: 10px 0; }
        .rpm-display { font-size: 1.2em; margin: 10px 0; }
    </style>
</head>
<body>
    <h1>Robot Control</h1>
    <div class="control-group">
        <h2>Motor Control</h2>
        <div class="rpm-display">Left RPM: <span id="leftRpm">0</span> | Right RPM: <span id="rightRpm">0</span></div>
        <button onclick="testMotors()">TEST MOTORS</button>
        <div class="slider-container">
            <label for="rpm">Speed (-255 to 255):</label>
            <input type="range" id="rpm" min="-255" max="255" value="0" oninput="updateRpm(this.value)">
            <span id="rpmValue">0</span>
        </div>
        <div class="slider-container">
            <label for="steering">Steering (-100 to 100):</label>
            <input type="range" id="steering" min="-100" max="100" value="0" oninput="updateSteering(this.value)">
            <span id="steeringValue">0</span>
        </div>
        <button onclick="stopMotors()">STOP</button>
    </div>
    <div class="control-group">
        <h2>Distance Sensors</h2>
        <p>Front: <span id="front" class="sensor-value">--</span> mm</p>
        <p>Left: <span id="left" class="sensor-value">--</span> mm</p>
        <p>Right: <span id="right" class="sensor-value">--</span> mm</p>
    </div>
    <div class="control-group">
        <h2>System Logs</h2>
        <div id="logs"></div>
    </div>
    <script>
        function updateRpm(value) {
            document.getElementById("rpmValue").textContent = value;
            fetch("/motors/rpm?value=" + value);  // Fix parameter passing
        }
        function updateSteering(value) {
            document.getElementById("steeringValue").textContent = value;
            fetch("/motors/steering?value=" + value);  // Fix parameter passing
        }
        function stopMotors() {
            fetch("/motors/stop");
            document.getElementById("rpm").value = 0;
            document.getElementById("rpmValue").textContent = "0";
            document.getElementById("steering").value = 0;
            document.getElementById("steeringValue").textContent = "0";
        }
        function updateSensors() {
            fetch("/distance")
                .then(response => response.json())
                .then(data => {
                    document.getElementById("front").textContent = data.front;
                    document.getElementById("left").textContent = data.left;
                    document.getElementById("right").textContent = data.right;
                });
        }
        function updateMotorStatus() {
            fetch("/motors/status")
                .then(response => response.text())
                .then(data => {
                    const [leftRpm, rightRpm] = data.split(",").map(s => parseInt(s));
                    document.getElementById("leftRpm").textContent = leftRpm;
                    document.getElementById("rightRpm").textContent = rightRpm;
                });
        }
        function updateLogs() {
            fetch("/log")
                .then(response => response.text())
                .then(data => {
                    const logs = document.getElementById("logs");
                    logs.innerHTML = data.replace(/\\n/g, "<br>");
                    logs.scrollTop = logs.scrollHeight;
                });
        }
        function testMotors() {
            fetch("/motors/test").then(response => {
                console.log("Motor test started");
            });
        }
        setInterval(updateSensors, 1000);
        setInterval(updateMotorStatus, 500);
        setInterval(updateLogs, 1000);
        stopMotors();
    </script>
</body>
</html>
)rawliteral";
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
        if(server.hasArg("value")) {  // Change parameter handling
            int steering = server.arg("value").toInt();
            if (steering >= -100 && steering <= 100) {
                motors.setSteering(steering / 100.0f);  // Convert to -1.0 to 1.0 range
                server.send(200, "text/plain", "Steering set to: " + String(steering));
            } else {
                server.send(400, "text/plain", "Invalid steering value");
            }
        } else {
            server.send(400, "text/plain", "Missing value parameter");
        }
    });

    server.on("/motors/rpm", HTTP_GET, [this]() {
        if(server.hasArg("value")) {  // Change parameter handling
            int rpm = server.arg("value").toInt();
            if (rpm >= -255 && rpm <= 255) {
                motors.setRpm(rpm);
                server.send(200, "text/plain", "RPM set to: " + String(rpm));
            } else {
                server.send(400, "text/plain", "Invalid RPM value");
            }
        } else {
            server.send(400, "text/plain", "Missing value parameter");
        }
    });

    server.on("/motors/stop", HTTP_GET, [this]() {
        motors.stop();
        server.send(200, "text/plain", "Motors stopped");
    });

    // Update motors/status endpoint to return just RPM values
    server.on("/motors/status", HTTP_GET, [this]() {
        String status = String(motors.getLeftRpm()) + "," + String(motors.getRightRpm());
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
