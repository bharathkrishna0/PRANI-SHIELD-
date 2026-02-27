#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// --- Configuration ---
const char* ssid = "Yks";
const char* password = "12345678";

// --- Pin Definitions ---
#define SOUND_PIN     D0
#define VIBRATION_PIN D1
#define SERVO_PIN     D2
#define RGB_RED       D3
#define LCD_SDA       D4
#define LCD_SCL       D5
#define BUTTON_PIN    D6  // RESET BUTTON
#define RGB_GREEN     D8
#define BUZZER_PIN    D9
#define RGB_BLUE      D10

// --- Objects ---
WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

// --- GLOBAL STATE ---
bool systemInAlertState = false;
bool lcdNeedsUpdate = true; // Flag to fix LCD blanking issue

// --- HTML DASHBOARD ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PRANI-SHIELD | Dashboard</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Roboto:wght@300;400;700&display=swap');
        :root { --bg-color: #0f172a; --glass-bg: rgba(15, 23, 42, 0.85); --glass-border: rgba(255, 255, 255, 0.1); --text-main: #e2e8f0; --text-dim: #94a3b8; --safe-color: #00ff88; --warning-color: #f59e0b; --danger-color: #ff0055; --accent-glow: rgba(0, 255, 136, 0.2); --grid-gap: 12px; }
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { background-color: var(--bg-color); background-image: linear-gradient(rgba(0, 0, 0, 0.6), rgba(0, 0, 0, 0.6)); color: var(--text-main); font-family: 'Roboto', sans-serif; height: 100vh; overflow: hidden; display: flex; justify-content: center; align-items: center; }
        .dashboard-container { width: 98vw; height: 96vh; display: grid; grid-template-columns: 300px 1fr 320px; grid-template-rows: auto 1fr auto; gap: var(--grid-gap); position: relative; }
        .glass-panel { background: var(--glass-bg); backdrop-filter: blur(12px); border: 1px solid var(--glass-border); border-radius: 10px; padding: 0.8rem; box-shadow: 0 4px 16px 0 rgba(0, 0, 0, 0.5); display: flex; flex-direction: column; gap: 0.5rem; }
        .header { grid-column: 1 / -1; display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--glass-border); padding-bottom: 0.5rem; margin-bottom: 0; }
        .brand-title { font-family: 'Orbitron', sans-serif; font-size: 1.8rem; font-weight: 900; letter-spacing: 3px; text-transform: uppercase; color: var(--safe-color); text-shadow: 0 0 10px var(--safe-color); }
        .system-status { font-family: 'Orbitron', sans-serif; font-size: 0.8rem; color: var(--safe-color); border: 1px solid var(--safe-color); padding: 0.25rem 0.75rem; border-radius: 4px; box-shadow: 0 0 5px var(--accent-glow); letter-spacing: 1px; }
        .main-status { grid-column: 2 / 3; grid-row: 2 / 3; display: flex; flex-direction: column; align-items: center; justify-content: center; position: relative; margin-top: -20px; }
        .status-ring { width: 220px; height: 220px; border-radius: 50%; border: 2px solid var(--glass-border); display: flex; align-items: center; justify-content: center; position: relative; box-shadow: 0 0 30px rgba(0, 0, 0, 0.5); transition: all 0.5s; }
        .status-ring::before { content: ''; position: absolute; top: -10px; bottom: -10px; left: -10px; right: -10px; border-radius: 50%; border: 2px solid transparent; border-top-color: var(--safe-color); border-bottom-color: var(--safe-color); animation: spin 8s linear infinite; }
        .main-alert-text { margin-top: 1.5rem; font-family: 'Orbitron', sans-serif; font-size: 2rem; font-weight: 700; text-transform: uppercase; letter-spacing: 3px; text-shadow: 0 0 15px rgba(0, 0, 0, 0.8); text-align: center; }
        .panel-title { font-family: 'Orbitron', sans-serif; font-size: 0.85rem; color: var(--text-dim); text-transform: uppercase; letter-spacing: 1px; border-bottom: 1px solid var(--glass-border); padding-bottom: 0.25rem; margin-bottom: 0.25rem; }
        .data-row { margin-bottom: 0.25rem; }
        .data-label { font-size: 0.7rem; color: var(--text-dim); margin-bottom: 0.1rem; text-transform: uppercase; letter-spacing: 1px; }
        .data-value { font-family: 'Orbitron', sans-serif; font-size: 1.1rem; color: var(--text-main); display: flex; justify-content: space-between; align-items: center; }
        .data-class { font-size: 0.7rem; font-weight: bold; padding: 1px 5px; border-radius: 4px; margin-left: 10px; }
        .class-normal { background: rgba(0, 255, 136, 0.1); color: var(--safe-color); border: 1px solid rgba(0, 255, 136, 0.3); }
        .class-abnormal { background: rgba(255, 0, 85, 0.2); color: var(--danger-color); border: 1px solid var(--danger-color); }
        .system-state-item { display: flex; justify-content: space-between; align-items: center; padding: 0.5rem; background: rgba(0, 0, 0, 0.2); border: 1px solid var(--glass-border); border-radius: 6px; }
        .state-name { font-size: 0.8rem; color: var(--text-dim); }
        .state-indicator { font-family: 'Orbitron', sans-serif; font-size: 0.8rem; font-weight: bold; color: var(--text-dim); }
        .state-active { color: var(--safe-color); text-shadow: 0 0 8px var(--safe-color); }
        .state-danger-active { color: var(--danger-color); text-shadow: 0 0 8px var(--danger-color); animation: blink 0.5s infinite alternate; }
        .bar-container { width: 100%; height: 5px; background: rgba(255, 255, 255, 0.1); border-radius: 3px; overflow: hidden; margin-top: 0.2rem; }
        .bar-fill { height: 100%; width: 0%; background: var(--safe-color); transition: width 0.3s ease, background-color 0.3s; box-shadow: 0 0 10px var(--safe-color); }
        .camera-panel { grid-column: 3 / 4; grid-row: 2 / 3; display: flex; flex-direction: column; }
        .camera-view { flex-grow: 1; background: #000; border-radius: 8px; display: flex; align-items: center; justify-content: center; position: relative; overflow: hidden; border: 1px solid var(--glass-border); min-height: 150px; }
        .camera-overlay { position: absolute; top: 8px; left: 8px; background: rgba(0, 0, 0, 0.7); padding: 3px 6px; border-radius: 4px; font-size: 0.65rem; color: var(--safe-color); display: flex; align-items: center; gap: 5px; z-index: 2; }
        .cam-alert-message { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); background: rgba(0, 0, 0, 0.85); border: 2px solid var(--danger-color); color: var(--danger-color); padding: 0.8rem 1.5rem; border-radius: 8px; font-family: 'Orbitron', sans-serif; font-weight: 900; font-size: 1rem; text-transform: uppercase; text-align: center; display: none; z-index: 10; box-shadow: 0 0 30px rgba(255, 0, 85, 0.5); animation: blink 0.5s infinite alternate; }
        .rec-dot { width: 6px; height: 6px; background: var(--danger-color); border-radius: 50%; animation: blink 1s infinite; display: none; }
        .rec-dot.active { display: block; }
        .camera-placeholder-text { color: var(--text-dim); font-family: 'Orbitron', sans-serif; text-align: center; font-size: 0.9rem; }
        .emergency-panel { display: none; margin-top: 0.5rem; padding: 0.5rem; border: 1px solid var(--danger-color); background: rgba(255, 0, 85, 0.1); border-radius: 8px; animation: slideIn 0.5s ease-out; }
        .emergency-title { color: var(--danger-color); font-weight: bold; font-family: 'Orbitron', sans-serif; font-size: 0.8rem; margin-bottom: 0.25rem; display: flex; align-items: center; gap: 0.5rem; }
        .phone-number { font-family: 'Orbitron', sans-serif; font-size: 1rem; color: #fff; margin-top: 0.25rem; display: block; }
        .messaging-tab { margin-top: 0.5rem; background: rgba(0, 0, 0, 0.3); border: 1px solid var(--glass-border); border-radius: 8px; padding: 0.5rem; }
        .msg-input { width: 100%; background: rgba(255, 255, 255, 0.05); border: 1px solid var(--glass-border); border-radius: 4px; color: #fff; padding: 5px; font-family: 'Roboto', sans-serif; resize: none; height: 40px; margin-bottom: 0.25rem; font-size: 0.8rem; }
        .msg-btn { width: 100%; background: var(--safe-color); border: none; border-radius: 4px; padding: 6px; font-family: 'Orbitron', sans-serif; font-weight: bold; color: #000; cursor: pointer; transition: all 0.3s; font-size: 0.8rem; }
        .msg-btn:hover { background: #fff; box-shadow: 0 0 15px var(--safe-color); }
        .msg-btn.alert-mode { background: var(--danger-color); color: #fff; }
        .msg-btn.alert-mode:hover { box-shadow: 0 0 15px var(--danger-color); }
        .visualizer-container { height: 60px; background: rgba(0, 0, 0, 0.3); border-radius: 6px; border: 1px solid var(--glass-border); overflow: hidden; position: relative; }
        canvas#bio-canvas { width: 100%; height: 100%; }
        .network-grid { margin-top: auto; border-top: 1px solid var(--glass-border); padding-top: 0.5rem; }
        .network-row { display: flex; justify-content: space-between; font-size: 0.7rem; color: var(--text-dim); margin-bottom: 0.2rem; }
        .net-status-dot { width: 5px; height: 5px; background: var(--safe-color); border-radius: 50%; display: inline-block; margin-right: 5px; box-shadow: 0 0 5px var(--safe-color); }
        .preventive-panel { grid-column: 2 / 3; grid-row: 3 / 4; display: none; animation: slideUp 0.5s ease-out; border: 1px solid var(--danger-color); background: rgba(255, 0, 85, 0.1); height: fit-content; }
        .preventive-list { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 0.5rem; }
        .measure-item { background: rgba(0, 0, 0, 0.3); padding: 0.5rem; border-radius: 8px; display: flex; align-items: center; gap: 0.5rem; border-left: 3px solid var(--danger-color); }
        .measure-icon { font-size: 1.2rem; }
        @keyframes spin { 100% { transform: rotate(360deg); } }
        @keyframes blink { 50% { opacity: 0; } }
        @keyframes slideUp { from { transform: translateY(20px); opacity: 0; } to { transform: translateY(0); opacity: 1; } }
        @keyframes slideIn { from { transform: translateX(20px); opacity: 0; } to { transform: translateX(0); opacity: 1; } }
        @keyframes pulse-red { 0% { box-shadow: 0 0 0 0 rgba(255, 0, 85, 0.7); } 70% { box-shadow: 0 0 0 30px rgba(255, 0, 85, 0); } 100% { box-shadow: 0 0 0 0 rgba(255, 0, 85, 0); } }
        .state-danger .brand-title, .state-danger .system-status, .state-danger .main-alert-text { color: var(--danger-color); text-shadow: 0 0 20px var(--danger-color); }
        .state-danger .system-status { border-color: var(--danger-color); }
        .state-danger .status-ring { border-color: var(--danger-color); box-shadow: 0 0 50px var(--danger-color); animation: pulse-red 1s infinite; }
        .state-danger .status-ring::before { border-top-color: var(--danger-color); border-bottom-color: var(--danger-color); animation-duration: 2s; }
        .state-danger .preventive-panel, .state-danger .emergency-panel, .state-danger .cam-alert-message { display: block; }
        .state-danger .camera-view { border-color: var(--danger-color); }
        .hidden { display: none; }
        svg { width: 100%; height: 100%; }
    </style>
</head>
<body class="state-safe">
    <div class="dashboard-container">
        <header class="header glass-panel">
            <div class="brand-title">PRANI-SHIELD</div>
            <div id="system-status" class="system-status">SYSTEM NORMAL</div>
        </header>

        <div class="glass-panel">
            <div class="panel-title">Sensor Array</div>
            <div class="data-row">
                <div class="data-label">Audio Input</div>
                <div class="data-value"><span id="sound-val">0</span> % <span id="sound-class" class="data-class class-normal">NORMAL</span></div>
                <div class="bar-container"><div id="sound-bar" class="bar-fill" style="width: 0%;"></div></div>
            </div>
            <div class="data-row">
                <div class="data-label">Seismic Activity</div>
                <div class="data-value"><span id="vib-val">0</span> % <span id="vib-class" class="data-class class-normal">STABLE</span></div>
                <div class="bar-container"><div id="vib-bar" class="bar-fill" style="width: 0%;"></div></div>
            </div>
            <div class="panel-title" style="margin-top: 0.5rem;">System Analysis</div>
            <div class="system-state-item"><span class="state-name">Optical Feed</span><span id="state-cam" class="state-indicator">STANDBY</span></div>
            <div class="system-state-item" style="margin-top:0.25rem"><span class="state-name">Neural Detection</span><span id="state-anim" class="state-indicator">SEARCHING</span></div>
            <div class="panel-title" style="margin-top: 0.5rem;">Bio-Signature</div>
            <div class="visualizer-container"><canvas id="bio-canvas"></canvas></div>
            <div class="data-row" style="margin-top:auto"><div class="data-label">Last Synchronization</div><div class="data-value" id="last-ping" style="font-size: 0.9rem;">--:--:--</div></div>
        </div>

        <div class="main-status">
            <div id="status-ring" class="status-ring">
                <div id="icon-safe" style="width: 100px; height: 100px;"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"></path><path d="M9 12l2 2 4-4"></path></svg></div>
                <div id="icon-danger" class="hidden" style="width: 100px; height: 100px;"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"></path><line x1="12" y1="9" x2="12" y2="13"></line><line x1="12" y1="17" x2="12.01" y2="17"></line></svg></div>
            </div>
            <div id="main-text" class="main-alert-text">SECURE</div>
        </div>

        <div class="camera-panel glass-panel">
            <div class="panel-title">Optical Feed</div>
            <div class="camera-view">
                <div id="cam-alert-overlay" class="cam-alert-message">CAMERA ACTIVATED<br><span style="font-size:0.7rem; font-weight:normal">ANALYZING FOOTAGE...</span></div>
                <div class="camera-overlay"><div id="rec-dot" class="rec-dot"></div><span id="cam-status-text">STANDBY</span></div>
                <div class="camera-placeholder-text">[ OPTICAL FEED OFFLINE ]<br><span style="font-size:0.8rem; opacity:0.6;">Awaiting Trigger</span></div>
            </div>
            <div id="emergency-panel" class="emergency-panel">
                <div class="emergency-title"><span style="animation:blink 1s infinite">⚠️</span> EMERGENCY SIGNAL</div>
                <div class="emergency-details">Signal broadcasting to nearest forest range tower.<br><strong>HQ:</strong> <span class="phone-number">+91 1800-425-4733</span></div>
            </div>
            <div class="panel-title" style="margin-top:0.5rem; border:none; margin-bottom:0.1rem">Direct Uplink</div>
            <div class="messaging-tab">
                <textarea id="hq-message" class="msg-input" placeholder="Type status report to Forest HQ..."></textarea>
                <button id="btn-send-msg" class="msg-btn" onclick="simulateSend()">SEND REPORT</button>
            </div>
            <div class="network-grid">
                <div class="panel-title" style="border:none; margin-bottom:0;">Forest Network</div>
                <div class="network-row"><span><i class="net-status-dot"></i> Tower Alpha</span><span style="color:var(--safe-color)">ONLINE</span></div>
                <div class="network-row"><span><i class="net-status-dot"></i> Tower Beta</span><span style="color:var(--safe-color)">ONLINE</span></div>
                <div class="network-row"><span><i class="net-status-dot" style="background:var(--warning-color); box-shadow:none;"></i> Satellite Up</span><span style="color:var(--warning-color)">LATENCY HIGH</span></div>
            </div>
        </div>

        <div class="preventive-panel glass-panel">
            <div class="panel-title" style="color:var(--danger-color); border-color:var(--danger-color);">⚠️ Threat Detected - Preventive Protocols</div>
            <div class="preventive-list">
                <div class="measure-item"><div class="measure-icon">🔊</div><div><div style="font-weight:bold; font-size:0.9rem;">Bee Emitter</div><div style="font-size:0.7rem;">Auto-Deploying...</div></div></div>
                <div class="measure-item"><div class="measure-icon">🔦</div><div><div style="font-weight:bold; font-size:0.9rem;">Strobe Lights</div><div style="font-size:0.7rem;">Active Pattern B</div></div></div>
                <div class="measure-item"><div class="measure-icon">📡</div><div><div style="font-weight:bold; font-size:0.9rem;">Ranger Alert</div><div style="font-size:0.7rem;">Sent to HQ</div></div></div>
            </div>
        </div>
    </div>

    <script>
        const body = document.body;
        const soundVal = document.getElementById('sound-val'); const soundClass = document.getElementById('sound-class'); const soundBar = document.getElementById('sound-bar');
        const vibVal = document.getElementById('vib-val'); const vibClass = document.getElementById('vib-class'); const vibBar = document.getElementById('vib-bar');
        const mainText = document.getElementById('main-text'); const systemStatus = document.getElementById('system-status');
        const iconSafe = document.getElementById('icon-safe'); const iconDanger = document.getElementById('icon-danger');
        const recDot = document.getElementById('rec-dot'); const camStatusText = document.getElementById('cam-status-text'); const lastPing = document.getElementById('last-ping');
        const stateCam = document.getElementById('state-cam'); const stateAnim = document.getElementById('state-anim');
        const bioCanvas = document.getElementById('bio-canvas'); const msgBtn = document.getElementById('btn-send-msg');
        const ctx = bioCanvas.getContext('2d');

        function resizeCanvas() { bioCanvas.width = bioCanvas.parentElement.clientWidth; bioCanvas.height = bioCanvas.parentElement.clientHeight; }
        window.addEventListener('resize', resizeCanvas); resizeCanvas();

        let frame = 0;
        function animateBio() {
            ctx.clearRect(0, 0, bioCanvas.width, bioCanvas.height);
            ctx.fillStyle = getComputedStyle(body).getPropertyValue(body.classList.contains('state-danger') ? '--danger-color' : '--safe-color');
            const bars = 20; const width = bioCanvas.width / bars;
            for (let i = 0; i < bars; i++) {
                let h = body.classList.contains('state-danger') ? Math.random() * bioCanvas.height : (Math.sin((frame + i) * 0.2) + 1) * 10 + 5;
                ctx.fillRect(i * width, (bioCanvas.height - h) / 2, width - 2, h);
            }
            frame++; requestAnimationFrame(animateBio);
        }
        animateBio();

        function simulateSend() {
            const originalText = msgBtn.innerText; msgBtn.innerText = "SENDING..."; msgBtn.disabled = true;
            setTimeout(() => { msgBtn.innerText = "SENT ✓"; setTimeout(() => { msgBtn.innerText = originalText; msgBtn.disabled = false; document.getElementById('hq-message').value = ''; }, 2000); }, 1000);
        }

        function updateUI(data) {
            data.time = new Date().toLocaleTimeString();

            soundVal.innerText = data.sound; soundBar.style.width = data.sound + '%';
            if (data.sound > 70) { soundClass.innerText = "ABNORMAL"; soundClass.className = "data-class class-abnormal"; soundBar.style.background = "var(--danger-color)"; } 
            else { soundClass.innerText = "NORMAL"; soundClass.className = "data-class class-normal"; soundBar.style.background = "var(--safe-color)"; }

            vibVal.innerText = data.vibration; 
            vibBar.style.width = data.vibration + '%';
            if (data.vibration > 50) { vibClass.innerText = "TREMOR"; vibClass.className = "data-class class-abnormal"; vibBar.style.background = "var(--danger-color)"; } 
            else { vibClass.innerText = "STABLE"; vibClass.className = "data-class class-normal"; vibBar.style.background = "var(--safe-color)"; }

            if (data.time) lastPing.innerText = data.time;

            if (data.alert) {
                recDot.classList.add('active'); camStatusText.innerText = "REC [EVENT]"; camStatusText.style.color = "var(--danger-color)";
                setDangerState();
            } else {
                recDot.classList.remove('active'); camStatusText.innerText = "STANDBY"; camStatusText.style.color = "var(--safe-color)";
                setSafeState();
            }
        }

        function setSafeState() {
            body.classList.remove('state-danger'); body.classList.add('state-safe');
            mainText.innerText = "SECURE"; systemStatus.innerText = "SYSTEM NORMAL";
            iconSafe.classList.remove('hidden'); iconDanger.classList.add('hidden');
            stateCam.innerText = "STANDBY"; stateCam.className = "state-indicator";
            stateAnim.innerText = "SEARCHING"; stateAnim.className = "state-indicator";
            msgBtn.classList.remove('alert-mode'); msgBtn.innerText = "SEND REPORT";
        }

        function setDangerState() {
            body.classList.remove('state-safe'); body.classList.add('state-danger');
            mainText.innerText = "ELEPHANT DETECTED"; systemStatus.innerText = "THREAT LEVEL: CRITICAL";
            iconSafe.classList.add('hidden'); iconDanger.classList.remove('hidden');
            stateCam.innerText = "ACTIVE"; stateCam.className = "state-indicator state-active";
            stateAnim.innerText = "PRESENCE DETECTED"; stateAnim.className = "state-indicator state-danger-active";
            msgBtn.classList.add('alert-mode'); msgBtn.innerText = "SEND EMERGENCY SOS";
        }

        async function fetchData() {
            try { const response = await fetch('/status'); const data = await response.json(); updateUI(data); } catch (e) { console.error(e); }
        }
        setInterval(fetchData, 1000);
    </script>
</body>
</html>
)rawliteral";

void handleRoot() { server.send(200, "text/html", index_html); }

void handleStatus() {
  int soundVal = analogRead(SOUND_PIN);
  int scaledSound, vibDisplay;

  // LOGIC TO FAKE HIGH NUMBERS DURING ALERT (Rising Animation)
  if (systemInAlertState) {
     scaledSound = random(80, 100); 
     vibDisplay = random(85, 100);   // Simulate rising high value
  } else {
     // REAL DATA (Scaled 0-100)
     scaledSound = map(soundVal, 0, 4095, 0, 100); 
     int rawVib = digitalRead(VIBRATION_PIN);
     vibDisplay = (rawVib == HIGH) ? 100 : 0; 
  }

  String json = "{";
  json += "\"sound\":" + String(scaledSound) + ",";
  json += "\"vibration\":" + String(vibDisplay) + ",";
  json += "\"alert\":" + String(systemInAlertState ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// --- HELPER: Non-blocking Delay that checks Reset Button ---
bool delayWithResetCheck(int ms) {
    for(int i = 0; i < ms / 10; i++) {
        server.handleClient(); // Keep WiFi UI alive
        if(digitalRead(BUTTON_PIN) == LOW) { return true; }
        delay(10);
    }
    return false; 
}

void setup() {
  Serial.begin(115200);

  // Pin Setup
  pinMode(SOUND_PIN, INPUT);
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RGB_RED, OUTPUT); pinMode(RGB_GREEN, OUTPUT); pinMode(RGB_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Servo Setup
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  myservo.write(0); // Default Open

  // LCD Setup - ADDED SAFETY DELAYS
  Wire.begin(LCD_SDA, LCD_SCL);
  delay(100); // Give I2C time to stabilize
  lcd.init(); 
  lcd.backlight();
  lcd.clear(); 
  
  // Set Initial Text
  lcd.setCursor(0, 0); lcd.print("Area Secured"); 
  lcd.setCursor(0, 1); lcd.print("Monitoring..."); 

  // WiFi Setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  Serial.print("IP: "); Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient(); 

  // --- 1. CHECK RESET BUTTON ---
  if (digitalRead(BUTTON_PIN) == LOW) {
      if(systemInAlertState) {
          systemInAlertState = false;
          lcdNeedsUpdate = true; // Flag for update
          myservo.write(0); // Open Servo (Gate)
          
          digitalWrite(RGB_RED, LOW); digitalWrite(RGB_GREEN, LOW); digitalWrite(RGB_BLUE, LOW); digitalWrite(BUZZER_PIN, LOW);
          delay(500); // Debounce
      }
  }

  // --- 2. CHECK VIBRATION ---
  int vibVal = digitalRead(VIBRATION_PIN);
  if (!systemInAlertState && vibVal == HIGH) {
      systemInAlertState = true; 
      lcdNeedsUpdate = true; // Flag for update
      myservo.write(180); // Close Servo (Max Rotation)
  }

  // --- 3. UPDATE LCD (Only on state change) ---
  if (lcdNeedsUpdate) {
      lcd.clear();
      if (systemInAlertState) {
          lcd.setCursor(0, 0); lcd.print("THREAT DETECTED!"); 
          lcd.setCursor(0, 1); lcd.print("GATE LOCKED");
      } else {
          lcd.setCursor(0, 0); lcd.print("Area Secured"); 
          lcd.setCursor(0, 1); lcd.print("Monitoring..."); 
      }
      lcdNeedsUpdate = false;
  }

  // --- 4. HANDLE STATES ---
  if (systemInAlertState) {
      // Siren Loop
      digitalWrite(RGB_RED, HIGH); digitalWrite(RGB_GREEN, LOW); digitalWrite(RGB_BLUE, LOW); digitalWrite(BUZZER_PIN, HIGH);
      if(delayWithResetCheck(200)) return;

      digitalWrite(RGB_RED, LOW); digitalWrite(RGB_GREEN, HIGH); digitalWrite(RGB_BLUE, LOW); digitalWrite(BUZZER_PIN, LOW);
      if(delayWithResetCheck(200)) return;

      digitalWrite(RGB_RED, LOW); digitalWrite(RGB_GREEN, LOW); digitalWrite(RGB_BLUE, HIGH); digitalWrite(BUZZER_PIN, HIGH);
      if(delayWithResetCheck(200)) return;
      
  } else {
      digitalWrite(RGB_RED, LOW); digitalWrite(RGB_GREEN, LOW); digitalWrite(RGB_BLUE, LOW); digitalWrite(BUZZER_PIN, LOW);
  }
}
