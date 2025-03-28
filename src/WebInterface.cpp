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
        nav { margin: 10px 0; }
        nav a { margin-right: 10px; }
        .control-input {
            margin: 10px 0;
        }
        .control-input input[type="number"] {
            width: 80px;
            margin: 0 10px;
        }
        .motor-stats {
            font-family: monospace;
            margin: 15px 0;
        }
        .motor-stats div {
            margin: 5px 0;
        }
        .rpm-mismatch {
            color: #ff4444;
        }
    </style>
</head>
<body>
    <nav>
        <a href="/">Home</a>
    </nav>
    <h1>Robot Control</h1>
    <div class="control-group">
        <h2>Mode Control</h2>
        <div>Mode: <span id="mode">OFF</span></div>
        <div>
            <button onclick="setMode('OFF')">OFF</button>
            <button onclick="setMode('MANUAL')">MANUAL</button>
            <button onclick="setMode('AUTO')">AUTO</button>
        </div>
    </div>
    <div class="control-group">
        <h2>Motor Control</h2>
        <div class="motor-stats">
            <div>Status: <span id="motorStatus">Stopped</span></div>
            <div>Stuck Detection: <span id="stuckStatus" style="font-weight: bold;">Unknown</span></div>
        </div>
        <button onclick="testMotors()">TEST MOTORS</button>
        <button onclick="calibrateMotors()">CALIBRATE</button>
        <button onclick="testBackup()">TEST BACKUP</button>
        <div id="calibrationStatus"></div>
        <div class="control-input">
            <label for="speed">Speed:</label>
            <input type="number" id="speed" value="0" min="-100" max="100">
            <span>(-100 to 100%)</span>
            <button onclick="setSpeed()">Set Speed</button>
        </div>
        <div class="control-input">
            <label for="steering">Steering (-100 to 100):</label>
            <input type="number" id="steering" min="-100" max="100" value="0">
            <button onclick="updateSteering()">Set Steering</button>
        </div>
        <button onclick="stopMotors()">STOP</button>
    </div>
    <div class="control-group">
        <h2>Distance Sensors</h2>
        <div>
            <label>
                <input type="checkbox" id="sensorUpdatesEnabled">
                Enable live updates
            </label>
        </div>
        <p>Front: <span id="front" class="sensor-value">--</span> mm</p>
        <p>Left: <span id="left" class="sensor-value">--</span> mm</p>
        <p>Right: <span id="right" class="sensor-value">--</span> mm</p>
    </div>
    <div class="control-group">
        <h2>System Logs</h2>
        <div>
            <label>
                <input type="checkbox" id="logsUpdatesEnabled">
                Enable live updates
            </label>
        </div>
        <div id="logs"></div>
    </div>
    <script>
        let lastSteeringUpdate = 0;
        const THROTTLE_MS = 100;  // Only send one command per 100ms
        function updateSteering(value) {
            document.getElementById("steeringValue").textContent = value;
            const now = Date.now();
            if (now - lastSteeringUpdate > THROTTLE_MS) {
                lastSteeringUpdate = now;
                fetch("/motors/steering?value=" + value);
                document.getElementById("steering").style.opacity = "0.7";
                setTimeout(() => document.getElementById("steering").style.opacity = "1", 100);
            }
        }
        function stopMotors() {
            fetch("/motors/stop");
            document.getElementById("speed").value = 0;
            document.getElementById("steering").value = 0;
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
        function testBackup() {
            fetch("/motors/test_backup").then(response => {
                console.log("Backup test started");
            });
        }
        function toggleMode() {
            fetch("/mode/toggle")
                .then(response => response.text())
                .then(mode => {
                    document.getElementById("mode").textContent = mode;
                    stopMotors();  // Always stop motors when changing modes
                });
        }
        function toggleState() {
            fetch("/toggle")
                .then(response => response.text())
                .then(state => {
                    document.getElementById("state").textContent = state;
                    if (state === "OFF") {
                        stopMotors();  // Stop motors when deactivating
                    }
                });
        }
        // Fix state update function to use 'mode' element instead of non-existent 'state'
        function updateState() {
            fetch("/status")
                .then(response => response.text())
                .then(state => {
                    document.getElementById("mode").textContent = state;
                });
        }
        function updateStuckStatus() {
            fetch("/status/stuck")
                .then(response => response.json())
                .then(data => {
                    const stuckElem = document.getElementById("stuckStatus");
                    if (data.stuck) {
                        stuckElem.textContent = "STUCK";
                        stuckElem.style.color = "#ff4444";
                    } else if (data.backupRemaining > 0) {
                        stuckElem.textContent = "Backing up (" + (data.backupRemaining/1000).toFixed(1) + "s)";
                        stuckElem.style.color = "#ff8800";
                    } else {
                        stuckElem.textContent = "Normal";
                        stuckElem.style.color = "#44aa44";
                    }
                });
        }
        let sensorUpdateInterval;
        let logsUpdateInterval;

        function startSensorUpdates() {
            updateSensors();
            updateStuckStatus();
            sensorUpdateInterval = setInterval(() => {
                updateSensors();
                updateStuckStatus();
            }, 250);
        }

        function stopSensorUpdates() {
            if (sensorUpdateInterval) {
                clearInterval(sensorUpdateInterval);
            }
        }

        function startLogsUpdates() {
            updateLogs(); // Update immediately
            logsUpdateInterval = setInterval(updateLogs, 1000);
        }

        function stopLogsUpdates() {
            if (logsUpdateInterval) {
                clearInterval(logsUpdateInterval);
            }
        }

        document.addEventListener('DOMContentLoaded', function() {
            updateSensors(); // Get initial sensor readings
            updateState();   // Get initial state
            setInterval(updateState, 1000);
            stopMotors();
            // Add event listeners for toggles
            document.getElementById('sensorUpdatesEnabled').addEventListener('change', function(e) {
                if (e.target.checked) {
                    startSensorUpdates();
                } else {
                    stopSensorUpdates();
                }
            });

            document.getElementById('logsUpdatesEnabled').addEventListener('change', function(e) {
                if (e.target.checked) {
                    startLogsUpdates();
                } else {
                    stopLogsUpdates();
                }
            });
        });

        function setMode(mode) {
            fetch("/mode/" + mode)
                .then(response => response.text())
                .then(newMode => {
                    document.getElementById("mode").textContent = newMode;
                    if (newMode !== "MANUAL") {
                        stopMotors();
                    }
                });
        }

        function setSpeed() {
            const speed = parseFloat(document.getElementById("speed").value);
            if (isNaN(speed) || speed < -100 || speed > 100) {
                alert("Speed must be between -100 and 100");
                return;
            }
            fetch("/motors/speed?value=" + speed);
        }

        function updateSteering() {
            const input = document.getElementById("steering");
            const value = parseInt(input.value);
            if (isNaN(value) || value < -100 || value > 100) {
                alert("Please enter a valid steering value between -100 and 100");
                return;
            }
            fetch("/motors/steering?value=" + value).then(() => {
                input.style.backgroundColor = "#e8ffe8";
                setTimeout(() => input.style.backgroundColor = "", 500);
            });
        }

        function calibrateMotors() {
            if (!confirm("Robot will perform calibration sequence. Continue?")) {
                return;
            }
            document.getElementById("calibrationStatus").textContent = "Calibrating...";
            fetch("/motors/calibrate")
                .then(response => response.json())
                .then(data => {
                    document.getElementById("calibrationStatus").textContent = 
                        `Calibration complete - Left scale: ${data.left}, Right scale: ${data.right}`;
                })
                .catch(error => {
                    document.getElementById("calibrationStatus").textContent = "Calibration failed!";
                });
        }
    </script>
    <style>
        .slider-container input[type="range"] {
            transition: opacity 0.1s;
        }
        /* Add smooth transitions for visual feedback */
        #leftPwm, #rightPwm {
            transition: color 0.2s;
        }
        .rpm-display span {
            display: inline-block;
            min-width: 3em;
        }
        #calibrationStatus {
            margin-top: 10px;
            font-family: monospace;
            color: #666;
        }
    </style>
</body>
</html>
)rawliteral";
        server.send(200, "text/html", html);
    });

    server.on("/status", HTTP_GET, [this]() {
        const char* mode;
        switch(robotState.getMode()) {
            case OperationMode::Off: mode = "OFF"; break;
            case OperationMode::Manual: mode = "MANUAL"; break;
            case OperationMode::Auto: mode = "AUTO"; break;
        }
        server.send(200, "text/plain", mode);
    });

    server.on("/toggle", HTTP_GET, [this]() {
        // Cycle through states: OFF -> MANUAL -> AUTO -> OFF
        switch(robotState.getMode()) {
            case OperationMode::Off:
                robotState.setMode(OperationMode::Manual);
                break;
            case OperationMode::Manual:
                robotState.setMode(OperationMode::Auto);
                break;
            case OperationMode::Auto:
                robotState.setMode(OperationMode::Off);
                break;
        }
        motors.stop();
        server.send(200, "text/plain", 
            robotState.isOff() ? "OFF" : 
            robotState.isManual() ? "MANUAL" : "AUTO");
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

    server.on("/motors/speed", HTTP_GET, [this]() {
        if (!robotState.isManual()) {
            server.send(400, "text/plain", "Must be in manual mode");
            return;
        }
        
        if(server.hasArg("value")) {
            float speed = server.arg("value").toFloat();
            if (speed >= -100 && speed <= 100) {
                robotState.resetActivityTimer();  // Reset inactivity timer
                motors.setSpeedPercent(speed);
                server.send(200, "text/plain", "OK");
            } else {
                server.send(400, "text/plain", "Invalid speed value");
            }
        }
    });

    server.on("/motors/steering", HTTP_GET, [this]() {
        if (!robotState.isManual()) {
            server.send(400, "text/plain", "Must be in manual mode");
            return;
        }

        if(server.hasArg("value")) {
            float steering = server.arg("value").toFloat();
            if (steering >= -1.0f && steering <= 1.0f) {
                robotState.resetActivityTimer();  // Reset inactivity timer
                motors.setSteering(steering);
                server.send(200, "text/plain", "OK");
            } else {
                server.send(400, "text/plain", "Invalid steering value");
            }
        }
    });

    server.on("/motors/stop", HTTP_GET, [this]() {
        motors.stop();
        server.send(200, "text/plain", "Motors stopped");
    });

    // Update motors/status endpoint to just return basic status:
    server.on("/motors/status", HTTP_GET, [this]() {
        String status = "Stopped";
        if (leftMotor.getCurrentSpeed() != 0 || rightMotor.getCurrentSpeed() != 0) {
            status = "Running";
        }
        server.send(200, "text/plain", status);
    });

    server.on("/mode/toggle", HTTP_GET, [this]() {
        if (robotState.isAuto()) {
            robotState.setMode(OperationMode::Manual);
        } else if (robotState.isManual()) {
            robotState.setMode(OperationMode::Off);
        } else {
            robotState.setMode(OperationMode::Auto);
        }
        motors.stop();
        server.send(200, "text/plain", 
            robotState.isOff() ? "OFF" : 
            robotState.isManual() ? "MANUAL" : "AUTO");
    });

    server.on("/log", HTTP_GET, [this]() {
        server.send(200, "text/plain", webLogger.getLogs());
    });

    // Update endpoint to handle mode changes
    server.on("/mode/OFF", HTTP_GET, [this]() {
        robotState.setMode(OperationMode::Off);
        motors.stop();
        server.send(200, "text/plain", "OFF");
    });

    server.on("/mode/MANUAL", HTTP_GET, [this]() {
        robotState.setMode(OperationMode::Manual);
        robotState.resetActivityTimer();  // Reset inactivity timer
        motors.stop();
        server.send(200, "text/plain", "MANUAL");
    });

    server.on("/mode/AUTO", HTTP_GET, [this]() {
        robotState.setMode(OperationMode::Auto);
        robot.resetStuckDetection(); // Reset stuck detection when switching to auto
        motors.stop();
        server.send(200, "text/plain", "AUTO");
    });

    // Add calibration endpoint
    server.on("/motors/calibrate", HTTP_GET, [this]() {
        if (!robotState.isManual()) {
            server.send(400, "text/plain", "Must be in manual mode");
            return;
        }
        motors.calibrate();
        String json = "{";
        json += "\"left\":" + String(motors.getLeftScale()) + ",";
        json += "\"right\":" + String(motors.getRightScale());
        json += "}";
        server.send(200, "application/json", json);
    });

    // Add new endpoint before the final curly brace
    server.on("/status/stuck", HTTP_GET, [this]() {
        String json = "{";
        json += "\"stuck\":" + String(robot.isStuck() ? "true" : "false") + ",";
        json += "\"backupRemaining\":" + String(robot.getBackupTimeRemaining());
        json += "}";
        server.send(200, "application/json", json);
    });

    server.on("/motors/test_backup", HTTP_GET, [this]() {
        robot.testBackup();
        server.send(200, "text/plain", "Running backup test");
    });
}
