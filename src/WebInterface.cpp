#include "WebInterface.h"

void WebInterface::begin() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = R"(
            <html><body style="font-family: Arial, sans-serif;">
            <h1>Robot Control</h1>
            <div style="margin: 20px 0;">
                Status: <span id="statusText">Checking...</span>
                <button id="toggleBtn" onclick='toggleRobot()' style="margin-left: 10px;">
                    Loading...
                </button>
            </div>
            <h2>Debug Menu</h2>
            <div style="margin: 20px 0;">
                <h3>Manual Control</h3>
                <div style="margin: 10px 0;">
                    <h4>Steering</h4>
                    <button onclick='fetch("/motors/steer/left").then(updateStatus)'>Turn Left</button>
                    <button onclick='fetch("/motors/steer/straight").then(updateStatus)'>Center</button>
                    <button onclick='fetch("/motors/steer/right").then(updateStatus)'>Turn Right</button>
                </div>
                <div style="margin: 10px 0;">
                    <h4>Drive</h4>
                    <button onclick='fetch("/motors/drive/forward").then(updateStatus)'>Forward</button>
                    <button onclick='fetch("/motors/drive/stop").then(updateStatus)'>STOP</button>
                    <button onclick='fetch("/motors/drive/backward").then(updateStatus)'>Backward</button>
                </div>
            </div>
            <div style="margin: 20px 0;">
                <h3>Test Sequence</h3>
                <button onclick='runTest()'>Run Full Test</button>
            </div>
            <div style="margin: 20px 0;">
                <h3>Control Mode</h3>
                <button id="modeBtn" onclick='toggleMode()'>Switch to Manual Mode</button>
            </div>
            <div id="motorStatus" style="margin: 10px 0; font-family: monospace;">
                Motor Status: Waiting...
            </div>
            <div id="sensorData" style="margin: 10px 0; font-family: monospace;">
                Loading sensor data...
            </div>
            <div style="margin: 20px 0;">
                <h3>Debug Log</h3>
                <div id="logArea" style="font-family: monospace; 
                                       white-space: pre; 
                                       background: #f0f0f0; 
                                       padding: 10px; 
                                       height: 400px; 
                                       overflow-y: scroll;
                                       font-size: 12px;
                                       line-height: 1.2;">
                </div>
            </div>
            <script>
                function updateStatus() {
                    fetch('/status')
                        .then(response => response.text())
                        .then(data => {
                            const status = data === 'ON';
                            document.getElementById('statusText').innerText = 
                                status ? 'Running' : 'Stopped';
                            document.getElementById('statusText').style.color = 
                                status ? 'green' : 'red';
                            document.getElementById('toggleBtn').innerText = 
                                status ? 'Stop Robot' : 'Start Robot';
                        });
                }
                
                function toggleRobot() {
                    fetch('/toggle')
                        .then(response => response.text())
                        .then(data => {
                            updateStatus();
                        });
                }
                
                function updateSensors() {
                    fetch('/sensors')
                        .then(response => response.text())
                        .then(data => {
                            document.getElementById('sensorData').innerHTML = 
                                data.replace(/\n/g, '<br>');
                        });
                }
                
                setInterval(updateSensors, 500);
                setInterval(updateStatus, 1000);
                updateSensors();
                updateStatus();

                async function runTest() {
                    const steps = [
                        ['/motors/drive/forward', 'Testing forward...'],
                        ['/motors/steer/right', 'Testing right turn...'],
                        ['/motors/steer/left', 'Testing left turn...'],
                        ['/motors/steer/straight', 'Testing straight...'],
                        ['/motors/drive/backward', 'Testing backward...'],
                        ['/motors/drive/stop', 'Test complete.']
                    ];
                    
                    for(const [url, msg] of steps) {
                        document.getElementById('motorStatus').innerText = msg;
                        await fetch(url);
                        await new Promise(resolve => setTimeout(resolve, 1000));
                    }
                }
                
                function updateMotorStatus(status) {
                    document.getElementById('motorStatus').innerText = 'Motor Status: ' + status;
                }

                function toggleMode() {
                    fetch('/mode/toggle')
                        .then(response => response.text())
                        .then(data => {
                            const isManual = data === 'MANUAL';
                            document.getElementById('modeBtn').innerText = 
                                isManual ? 'Switch to Auto Mode' : 'Switch to Manual Mode';
                            document.getElementById('modeBtn').style.backgroundColor = 
                                isManual ? '#ffeb3b' : '#4caf50';
                        });
                }

                function updateLog() {
                    fetch('/log')
                        .then(response => response.text())
                        .then(data => {
                            const logArea = document.getElementById('logArea');
                            const wasScrolledToBottom = 
                                logArea.scrollHeight - logArea.clientHeight <= logArea.scrollTop + 1;
                            logArea.textContent = data;
                            if (wasScrolledToBottom) {
                                logArea.scrollTop = logArea.scrollHeight;
                            }
                        });
                }
                setInterval(updateLog, 1000);
                updateLog();
            </script>
            </body></html>
        )";
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

    server.on("/sensors", HTTP_GET, [this](AsyncWebServerRequest *request){
        sensors.read();
        String response = "Front: " + String(sensors.getFrontDistance()) + "mm\n"
                         "Left: " + String(sensors.getLeftDistance()) + "mm\n"
                         "Right: " + String(sensors.getRightDistance()) + "mm";
        request->send(200, "text/plain", response);
    });

    // New motor control endpoints
    server.on("/motors/steer/left", HTTP_GET, [this](AsyncWebServerRequest *request){
        motors.setSteering(LEFT);
        request->send(200, "text/plain", "Steering: LEFT");
    });

    server.on("/motors/steer/right", HTTP_GET, [this](AsyncWebServerRequest *request){
        motors.setSteering(RIGHT);
        request->send(200, "text/plain", "Steering: RIGHT");
    });

    server.on("/motors/steer/straight", HTTP_GET, [this](AsyncWebServerRequest *request){
        motors.setSteering(STRAIGHT);
        request->send(200, "text/plain", "Steering: STRAIGHT");
    });

    server.on("/motors/drive/forward", HTTP_GET, [this](AsyncWebServerRequest *request){
        motors.setSpeed(100);
        request->send(200, "text/plain", "Driving forward");
    });

    server.on("/motors/drive/backward", HTTP_GET, [this](AsyncWebServerRequest *request){
        motors.setSpeed(-100);
        request->send(200, "text/plain", "Driving backward");
    });

    server.on("/motors/drive/stop", HTTP_GET, [this](AsyncWebServerRequest *request){
        motors.stop();
        request->send(200, "text/plain", "Stopped");
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
