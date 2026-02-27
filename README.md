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
