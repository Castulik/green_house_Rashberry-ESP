# 🌱 IoT Smart Greenhouse Monitor

**A comprehensive system for remote greenhouse monitoring using LoRa, ESP-NOW, and MQTT.**

This project solves a classic problem:*"How do I get data from a greenhouse that is far from the house and out of Wi-Fi range, without having to change sensor batteries every two days?"*

![Architecture Diagram](diagram.svg)

📊 Dashboard Preview 
The application provides an overview of current temperature, min/max values, and historical trends.

<img src="UI.png" width="90%" height="70%">

## 🚀 How It Works (Architecture)

The system utilizes a three-layer architecture to maximize range and minimize energy consumption:

1.  **Sensor Layer (ESP32-C3 + ESP-NOW):**
    * Sensors inside the greenhouse measure temperature and humidity.
    * They use the ESP-NOW protocol for lightning-fast data transmission and immediate transition to Deep Sleep. This allows them to run on batteries for months.
2.  **Bridging Layer (ESP32-S3 + LoRa):**
    * A "Hub" located in the greenhouse (e.g., on the roof) receives data from the sensors.
    * It immediately forwards them using LoRa (Long Range) technology towards the house. LoRa ensures signal penetration through walls and over hundreds of meters.
3.  **Gateway & Cloud (ESP32-S3 + MQTT):**
    * The receiver in the house captures the LoRa signal, connects to the home Wi-Fi, and publishes data to an MQTT Broker (e.g., Shiftr.io).
4.  **Visualization (Python + Streamlit):**
    * The server side (Python script) stores data in an SQLite database.
    * The web application (Streamlit) displays interactive charts and statistics.

---

## 🛠 Hardware Used

* **Senzor:** ESP32-C3 (nebo ESP8266) + DHT22 / BME280.
* **Hub (Greenhouse):** ESP32-S3 + LoRa module (SX1278 / SX1262).
* **Gateway (Home):** ESP32-S3 + LoRa module.
* **Server:** Raspberry Pi / PC / Cloud for running Python scripts.

---

## ⚙️ Configuration and Installation

### 1. Firmware (ESP32)
In the `/firmware_esp` folder, you will find three different codes for the three devices. Before flashing, **you must update the configuration credentials** in the `Gateway.ino` and `database.py` files:

```cpp
// EXAMPLE SETTINGS IN CODE (Gateway.ino)
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Settings (Register e.g. on shiftr.io)
const char* mqtt_server = "public.cloud.shiftr.io";
const char* mqtt_user = "your_username";
const char* mqtt_pass = "your_password_or_token";
```

### 2. Backend (Python Logger)

This script must run continuously in the background to collect data from MQTT.

### Install dependencies:

```bash
pip install paho-mqtt
```
Update credentials in database.py (same as in ESP32).

Run the logger:
```bash
python logger.py
```

3. Frontend (Streamlit Dashboard)
Application for data visualization.

Install dependencies:
```bash
pip install streamlit pandas plotly streamlit-autorefresh
```

Run the application:
```bash
streamlit run app.py
```

### 📁 Project Structure

* /firmware      - Source codes for ESP32 (Arduino IDE / PlatformIO)
* /src           - Python scripts (logger.py, app.py)
* diagram.svg    - Wiring/Logic diagram
* teplota.db     - SQLite database (created automatically upon launch)

### ⚠️ Disclaimer
This is a hobby project. Ensure that your LoRa modules transmit on a frequency permitted in your country (typically 868 MHz or 433 MHz in the EU).

Created by [Filip Častulík]