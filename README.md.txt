#  Environment and Safety Monitoring System

## 📌 Project Overview
This project is an IoT-based Environment and Safety Monitoring System designed for a smart mine project. It continuously tracks local microclimate conditions and features an emergency alert system to protect mine personnel.

## ⚙️ Core Features
* **Ambient Telemetry:** Uses a DHT22 sensor to track real-time temperature and humidity levels within the mine.
* **Instant Distress Line:** Incorporates an interrupt-driven hardware SOS button for immediate emergency signaling.
* **Cloud Logging:** Sends sensor data and critical emergency triggers directly to a ThingSpeak dashboard.
* **Network Resilience:** Automatically reconnects to Wi-Fi in the background without freezing the monitoring functions.

## 🛠️ Hardware & Tools Used
* ESP32 Microcontroller
* DHT22 Temperature & Humidity Sensor
* Push Button (SOS Alert)
* Arduino IDE (Firmware Development)
* ThingSpeak API (IoT Cloud Platform)
