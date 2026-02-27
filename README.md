# 🛡️ PRANI-SHIELD  
### Predictive • Responsive • Autonomous • Non-lethal • Intelligent Defense

> Smart, low-cost AI-based wildlife intrusion prevention system for rural and forest-border communities.

---

## 🌿 Overview

**PRANI-SHIELD** is a distributed, edge-AI powered wildlife detection and alert system designed to reduce human–wildlife conflict in rural areas.

The system integrates:

- 🧠 Multi-sensor detection  
- 👁️ AI-based visual verification (Raspberry Pi Camera + Grove Vision AI)  
- 🔐 Servo-driven smart latch (gate control)  
- 🌐 Local ESP32 web dashboard  
- 🔋 Low-power battery-based deployment  

Unlike electric fencing, PRANI-SHIELD is **non-lethal, ethical, affordable, and scalable**.

---

## 🎯 Problem Statement

Rural communities near forests face:

- Crop destruction  
- Human injuries  
- Wildlife fatalities  
- Unsafe electric fencing solutions  

Existing systems are:
- Expensive  
- Cloud-dependent  
- Power-hungry  
- Harmful to animals  

PRANI-SHIELD provides an **offline, intelligent, and humane alternative**.

---

## 🧩 System Architecture

### 🔹 1️⃣ Distributed Sensor Nodes
- Vibration sensor  
- Sound sensor  
- ESP32 controller  
- Deep sleep enabled  
- Battery powered  

These nodes detect abnormal movement and trigger camera verification.

---

### 🔹 2️⃣ AI Vision Node (Selective Deployment)

- Raspberry Pi Camera Rev 1.3  
- Grove Vision AI Module  
- ESP32-S3 (communication + control)  
- Event-triggered activation  

Only activated when sensor nodes detect suspicious activity.

---

### 🔹 Detection Flow

Sensor Trigger
↓
Camera Activation
↓
AI Verification (Animal / Human)
↓
Alert System + Gate Control

### 🔹 3️⃣ Actuation & Alerts

- SG90 Servo → Gate/Latch Lock  
- RGB LED → Visual Alert  
- Active Buzzer → Audible Warning  
- I2C 16x2 LCD → Local Status Display  
- Web Dashboard → Remote Monitoring  

---

## 🧠 Edge AI Integration

The Raspberry Pi Camera connects to the Grove Vision AI module.

- Runs object detection locally  
- Detects animals (e.g., elephants)  
- Sends bounding box + class data via UART  
- Works fully offline  

This ensures:
- Low latency  
- No internet dependency  
- Reduced false alarms  

---

## 🔌 Hardware Used

| Component | Purpose |
|------------|----------|
| ESP32 / ESP32-S3 | Control & WiFi |
| Raspberry Pi Camera Rev 1.3 | Image Capture |
| Grove Vision AI | Edge AI Inference |
| Vibration Sensor | Footstep Detection |
| Sound Sensor | Audio Activity Detection |
| SG90 Servo | Smart Latch Control |
| RGB LED | Alert Indicator |
| Active Buzzer | Audible Alert |
| I2C 16x2 LCD | Local Status Display |
| 18650 Li-ion Battery | Portable Power |

---

## 🌐 Web Dashboard

Hosted locally on ESP32.

Features:
- Real-time sensor monitoring  
- Threat detection status  
- Alert visualization  
- Camera activation indicator  
- JSON API endpoint (`/status`)  

Works fully offline via local WiFi.

---

## 🔋 Power Optimization Strategy

- Deep sleep for sensor nodes  
- Event-driven camera activation  
- No continuous streaming  
- Battery-optimized design  
- Solar-ready architecture  

---

## 💰 Cost Efficiency (Rural-Focused)

| Node Type | Approx Cost (INR) |
|------------|------------------|
| Sensor Node | ₹600–₹700 |
| AI Vision Node | ₹3500–₹4000 |

Shared deployment reduces per-household cost significantly.

---

## 🚀 Future Scope

- Anti-poaching monitoring  
- Wildlife corridor analytics  
- Forest perimeter security  
- Solar-powered deployment  
- Predictive conflict hotspot mapping  

---

## 🛠️ Setup Instructions

### 1️⃣ Upload Firmware
Flash ESP32 using Arduino IDE.

### 2️⃣ Configure WiFi
```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
