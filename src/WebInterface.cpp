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
        <a href="/encoders">Encoder Status</a>
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
        </div>
        <button onclick="testMotors()">TEST MOTORS</button>
        <div class="control-input">
            <label for="speed">Speed (RPM):</label>
            <input type="number" id="speed" value="0" min="-200" max="200">
            <span>(-200 to 200 RPM)</span>
            <button onclick="updateSpeed()">Set Speed</button>
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
        <p>Front: <span id="front" class="sensor-value">--</span> mm</p>
        <p>Left: <span id="left" class="sensor-value">--</span> mm</p>
        <p>Right: <span id="right" class="sensor-value">--</span> mm</p>
    </div>
    <div class="control-group">
        <h2>System Logs</h2>
        <div id="logs"></div>
    </div>
    <script>
        let lastRpmUpdate = 0;
        let lastSteeringUpdate = 0;
        const THROTTLE_MS = 100;  // Only send one command per 100ms

        function updateRpm() {
            const input = document.getElementById("rpm");
            const value = parseInt(input.value);
            if (isNaN(value) || value < -255 || value > 255) {
                alert("Please enter a valid speed between -255 and 255");
                return;
            }
            
            document.getElementById("rpmValue").textContent = value;
            fetch("/motors/rpm?value=" + value).then(() => {
                input.style.backgroundColor = "#e8ffe8";
                setTimeout(() => input.style.backgroundColor = "", 500);
            });
        }

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

        // Add state update to periodic updates
        function updateState() {
            fetch("/status")
                .then(response => response.text())
                .then(state => {
                    document.getElementById("state").textContent = state;
                });
        }

        // Add keyboard event listener for the speed input
        document.getElementById("rpm").addEventListener("keypress", function(e) {
            if (e.key === "Enter") {
                e.preventDefault();
                updateRpm();
            }
        });

        setInterval(updateState, 1000);
        setInterval(updateSensors, 2000);    // Sensors every 2s
        setInterval(updateLogs, 1000);        // Logs every second
        stopMotors();

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

        function updateSpeed() {
            const value = parseInt(document.getElementById("speed").value);
            if (isNaN(value) || value < -200 || value > 200) {
                alert("Please enter a valid RPM between -200 and 200");
                return;
            }
            fetch("/motors/rpm?value=" + value);
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

        document.querySelectorAll('input[name="controlMode"]').forEach(input => {
            input.addEventListener('change', updateControlMode);
        });
        updateControlMode();
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

    // For parameters, use URI patterns with WebServer
    server.on("/motors/steering", HTTP_GET, [this]() {
        if (robot.isAuto()) {
            server.send(400, "text/plain", "Must be in manual mode");
            return;
        }
        
        if(server.hasArg("value")) {
            int steering = server.arg("value").toInt();
            if (steering >= -100 && steering <= 100) {
                // Convert to proper steering range and invert for intuitive control
                // 0 = straight, positive = right turn, negative = left turn
                motors.setSteering(-steering / 100.0f);
                server.send(200, "text/plain", "OK");
            } else {
                server.send(400, "text/plain", "Invalid steering value");
            }
        }
    });

    server.on("/motors/rpm", HTTP_GET, [this]() {
        if (!robotState.isManual()) {
            server.send(400, "text/plain", "Must be in manual mode");
            return;
        }
        
        if(server.hasArg("value")) {
            float rpm = server.arg("value").toFloat();
            if (rpm >= -MOTOR_MAX_RPM && rpm <= MOTOR_MAX_RPM) {
                motors.setRPM(rpm);
                server.send(200, "text/plain", "RPM set to: " + String(rpm));
            } else {
                server.send(400, "text/plain", "Invalid RPM value");
            }
        }
    });

    server.on("/motors/stop", HTTP_GET, [this]() {
        motors.stop();  // Just stop motors, no manual control needed
        server.send(200, "text/plain", "Motors stopped");
    });

    // Update motors/status endpoint to just return basic status:
    server.on("/motors/status", HTTP_GET, [this]() {
        String status = "Stopped";
        if (leftMotor.getCurrentRPM() != 0 || rightMotor.getCurrentRPM() != 0) {
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

    // Replace debug endpoint with encoders
    server.on("/encoders", HTTP_GET, [this]() {
        const char* html = R"rawliteral(
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; }
        .diagnostic { margin: 10px 0; }
        #motorData { font-family: monospace; }
        nav { margin: 10px 0; }
        nav a { margin-right: 10px; }
    </style>
</head>
<body>
    <nav>
        <a href="/">Home</a>
        <a href="/encoders">Encoder Status</a>
    </nav>
    <h1>Encoder Status</h1>
    <div id="motorData">Loading...</div>
    <script>
        function updateMotorData() {
            fetch('/motor-diagnostics')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('motorData').innerHTML = 
                        `<pre>Left Motor:
  Pulse Count: ${data.left.pulseCount}
  Last Pulse: ${data.left.lastPulse}ms ago
  Current PWM: ${data.left.pwm}

Right Motor:
  Pulse Count: ${data.right.pulseCount}
  Last Pulse: ${data.right.lastPulse}ms ago
  Current PWM: ${data.right.pwm}</pre>`;
                });
        }
        //setInterval(updateMotorData, 1000);
    </script>
</body>
</html>
)rawliteral";
        server.send(200, "text/html", html);
    });

    // Add diagnostics endpoint
    server.on("/motor-diagnostics", HTTP_GET, [this]() {
        String json = "{";
        json += "\"left\":{";
        json += "\"pulseCount\":" + String(leftMotor.getPulseCount()) + ",";
        json += "\"lastPulse\":" + String(leftMotor.getTimeSinceLastPulse()) + ",";
        json += "\"pwm\":" + String(leftMotor.getCurrentPwm());
        json += "},";
        json += "\"right\":{";
        json += "\"pulseCount\":" + String(rightMotor.getPulseCount()) + ",";
        json += "\"lastPulse\":" + String(rightMotor.getTimeSinceLastPulse()) + ",";
        json += "\"pwm\":" + String(rightMotor.getCurrentPwm());
        json += "}}";
        server.send(200, "application/json", json);
    });

    // Update endpoint to handle mode changes
    server.on("/mode/OFF", HTTP_GET, [this]() {
        robotState.setMode(OperationMode::Off);
        motors.stop();
        server.send(200, "text/plain", "OFF");
    });

    server.on("/mode/MANUAL", HTTP_GET, [this]() {
        robotState.setMode(OperationMode::Manual);
        motors.stop();
        server.send(200, "text/plain", "MANUAL");
    });

    server.on("/mode/AUTO", HTTP_GET, [this]() {
        robotState.setMode(OperationMode::Auto);
        motors.stop();
        server.send(200, "text/plain", "AUTO");
    });
}
