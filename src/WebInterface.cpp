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
        <div>
            Mode: <span id="mode">MANUAL</span> | 
            State: <span id="state">INACTIVE</span>
        </div>
        <div>
            <button onclick="toggleMode()">Toggle Mode (Manual/Auto)</button>
            <button onclick="toggleState()">Toggle State (Active/Inactive)</button>
        </div>
    </div>
    <div class="control-group">
        <h2>Motor Control</h2>
        <div class="rpm-display">Left PWM: <span id="leftPwm">0</span> | Right PWM: <span id="rightPwm">0</span></div>
        <button onclick="testMotors()">TEST MOTORS</button>
        <div class="slider-container">
            <label for="rpm">Speed (-255 to 255):</label>
            <input type="range" id="rpm" min="-255" max="255" value="0" step="16" oninput="updateRpm(this.value)">
            <span id="rpmValue">0</span>
        </div>
        <div class="slider-container">
            <label for="steering">Steering (-100 to 100):</label>
            <input type="range" id="steering" min="-100" max="100" value="0" step="5" oninput="updateSteering(this.value)">
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
        let lastRpmUpdate = 0;
        let lastSteeringUpdate = 0;
        const THROTTLE_MS = 100;  // Only send one command per 100ms

        function updateRpm(value) {
            document.getElementById("rpmValue").textContent = value;
            
            const now = Date.now();
            if (now - lastRpmUpdate > THROTTLE_MS) {
                lastRpmUpdate = now;
                fetch("/motors/rpm?value=" + value);
                document.getElementById("rpm").style.opacity = "0.7";
                setTimeout(() => document.getElementById("rpm").style.opacity = "1", 100);
            }
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
        function updateMotorStatus() {
            fetch("/motors/status")
                .then(response => response.text())
                .then(data => {
                    const [leftPwm, rightPwm] = data.split(",").map(s => parseInt(s));
                    document.getElementById("leftPwm").textContent = leftPwm;
                    document.getElementById("rightPwm").textContent = rightPwm;
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

        setInterval(updateState, 1000);
        setInterval(updateSensors, 2000);    // Sensors every 2s
        setInterval(updateMotorStatus, 200);  // Motor status 5x per second
        setInterval(updateLogs, 1000);        // Logs every second
        stopMotors();
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
        server.send(200, "text/plain", robot.isAuto() ? "AUTO" : "MANUAL");
    });

    server.on("/toggle", HTTP_GET, [this]() {
        robot.toggleMode();
        server.send(200, "text/plain", robot.isAuto() ? "AUTO" : "MANUAL");
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
        if (robot.isAuto()) {
            server.send(400, "text/plain", "Must be in manual mode");
            return;
        }
        
        if(server.hasArg("value")) {
            int pwm = server.arg("value").toInt();
            if (pwm >= -255 && pwm <= 255) {
                motors.setPwm(pwm);
                server.send(200, "text/plain", "PWM set to: " + String(pwm));
            } else {
                server.send(400, "text/plain", "Invalid PWM value");
            }
        }
    });

    server.on("/motors/stop", HTTP_GET, [this]() {
        motors.stop();  // Just stop motors, no manual control needed
        server.send(200, "text/plain", "Motors stopped");
    });

    // Update motors/status endpoint to return just PWM values
    server.on("/motors/status", HTTP_GET, [this]() {
        String status = String(leftMotor.getCurrentPwm()) + "," + String(rightMotor.getCurrentPwm());
        server.send(200, "text/plain", status);
    });

    server.on("/mode/toggle", HTTP_GET, [this]() {
        bool isManual = robot.toggleMode();
        motors.stop();  // Always stop motors when changing modes
        server.send(200, "text/plain", isManual ? "MANUAL" : "AUTO");
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
        setInterval(updateMotorData, 100);
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
}
