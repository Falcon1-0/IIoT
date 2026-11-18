# Multi-Site Industrial IoT Shop Floor Monitoring System

## Overview

This project implements a **Multi-Site Industrial IoT (IIoT) Monitoring System** designed to monitor environmental and operational conditions across three distributed manufacturing shop floors.

Each shop floor independently collects sensor data using an edge device and transmits the measurements to **Blynk IoT Cloud** using MQTT. The cloud platform provides centralized visualization of all three locations and warning indicators when predefined operating limits are exceeded.

The prototype demonstrates how IIoT can provide centralized visibility, remote monitoring, and early detection of abnormal conditions across geographically distributed manufacturing facilities.

---

## Project Objectives

- Model three distributed manufacturing shop floors.
- Collect real-time temperature, humidity, and air-pressure data.
- Use Raspberry Pi 4 and ESP32 devices for edge data acquisition.
- Transmit sensor data using MQTT.
- Integrate all three locations with Blynk IoT Cloud.
- Provide centralized web/mobile monitoring.
- Detect abnormal conditions using predefined thresholds.
- Generate warning indicators for operators.

---

## System Architecture

The overall system follows the following data flow:

```text
Shop Floor Sensors
        |
        v
   Edge Devices
(Raspberry Pi / ESP32)
        |
        v
      MQTT
        |
        v
 Blynk IoT Cloud
        |
        v
Web / Mobile Dashboard
        |
        v
Monitoring & Alerts
```

---

## Hardware Used

### Factory 1

- Raspberry Pi 4
- Raspberry Pi Sense HAT
- Wi-Fi connection

The Sense HAT provides:

- Temperature
- Humidity
- Atmospheric Pressure

### Factory 2

- Raspberry Pi 4
- Raspberry Pi Sense HAT
- Wi-Fi connection

The Sense HAT provides:

- Temperature
- Humidity
- Atmospheric Pressure

### Factory 3

- ESP32
- DHT11 Temperature and Humidity Sensor
- Wi-Fi connection

The ESP32 provides:

- Temperature using DHT11
- Humidity using DHT11
- Simulated Atmospheric Pressure

> Pressure is simulated for Factory 3 because an external pressure sensor was not available for the ESP32 prototype.

---

## Software and Technologies

| Technology | Purpose |
|---|---|
| Python | Raspberry Pi programming |
| Arduino/C++ | ESP32 programming |
| MQTT | IIoT communication |
| MQTT over TLS | Secure cloud communication |
| Blynk IoT Cloud | Central monitoring platform |
| Sense HAT | Environmental sensing |
| DHT11 | Temperature and humidity sensing |
| Paho MQTT | Raspberry Pi MQTT client |
| PubSubClient | ESP32 MQTT client |
| Wi-Fi | Network connectivity |

---

## Repository Structure

```text
Multi-Site-IIoT-Monitoring/
│
├── Factory1.py
├── Factory2.py
├── Factory3.ino
└── README.md
```

### Factory1.py

Runs on the Raspberry Pi at Factory 1.

The program:

- Reads temperature from the Sense HAT.
- Reads humidity from the Sense HAT.
- Reads atmospheric pressure from the Sense HAT.
- Evaluates warning conditions.
- Creates a JSON payload.
- Publishes the data to Blynk using MQTT over TLS.

### Factory2.py

Runs on the Raspberry Pi at Factory 2.

It performs the same monitoring process independently for the second manufacturing location.

### Factory3.ino

Runs on the ESP32 at Factory 3.

The program:

- Connects the ESP32 to Wi-Fi.
- Reads temperature from the DHT11.
- Reads humidity from the DHT11.
- Generates a simulated pressure value.
- Evaluates warning conditions.
- Creates a JSON payload.
- Publishes the measurements to Blynk using MQTT.

---

## Monitored Parameters

The prototype monitors three parameters:

### Temperature

Temperature monitoring helps identify abnormal thermal conditions within the manufacturing environment.

### Humidity

Humidity monitoring helps identify environmental conditions that may affect manufacturing processes or equipment.

### Air Pressure

Atmospheric pressure provides an additional environmental parameter for multi-sensor IIoT monitoring.

Factory 1 and Factory 2 obtain actual pressure measurements from the Sense HAT.

Factory 3 uses simulated pressure because an external pressure sensor was not available.

---

## Warning Thresholds

The prototype uses the following thresholds:

| Parameter | Warning Threshold |
|---|---:|
| Temperature | > 35 °C |
| Humidity | > 50% |
| Air Pressure | > 1020 hPa |

The system assigns a warning code based on the detected condition:

| Warning Code | Status |
|---:|---|
| 0 | Normal |
| 1 | High Temperature |
| 2 | High Humidity |
| 3 | High Air Pressure |

### Warning Logic

```text
Temperature > 35 °C
        |
        v
   Warning = 1

Otherwise

Humidity > 50%
        |
        v
   Warning = 2

Otherwise

Pressure > 1020 hPa
        |
        v
   Warning = 3

Otherwise
        |
        v
Warning = 0 (Normal)
```

---

## Blynk Datastreams

The sensor readings are mapped to the following Blynk datastreams:

| Datastream | Virtual Pin | Description |
|---|---|---|
| Temperature | V0 | Temperature measurement |
| Humidity | V1 | Relative humidity |
| Air Pressure | V2 | Atmospheric pressure |
| Warning | V6 | Warning status code |

The devices transmit the measurements using a JSON payload.

### Example Payload

```json
{
  "Temperature": 26.42,
  "Humidity": 44.31,
  "Air Pressure": 978.52,
  "Warning": 0
}
```

The payload is published to:

```text
batch_ds
```

---

## Data Flow

Each edge device follows the same general process:

```text
Read Sensors
     |
     v
Process Measurements
     |
     v
Compare Against Thresholds
     |
     v
Generate Warning Code
     |
     v
Create JSON Payload
     |
     v
Publish Through MQTT
     |
     v
Blynk IoT Cloud
     |
     v
Dashboard Visualization
```

Sensor data is transmitted approximately every **3 seconds**, providing near-real-time monitoring.

---

## Raspberry Pi Implementation

The Raspberry Pi uses the Sense HAT to obtain environmental measurements:

```python
temperature = sense.get_temperature_from_humidity()
humidity = sense.get_humidity()
pressure = sense.get_pressure()
```

The measurements are placed into a JSON-compatible payload:

```python
payload = {
    "Temperature": round(temperature, 2),
    "Humidity": round(humidity, 2),
    "Air Pressure": round(pressure, 2),
    "Warning": warning
}
```

The payload is published to Blynk using MQTT:

```python
message = client.publish(
    "batch_ds",
    json.dumps(payload),
    qos=1
)
```

---

## ESP32 Implementation

The ESP32 uses a DHT11 sensor for temperature and humidity:

```cpp
float temperature = dht.readTemperature();
float humidity = dht.readHumidity();
```

Because an external pressure sensor was unavailable, the pressure value is simulated.

```cpp
float pressure = random(97000, 98001) / 100.0f;
```

The ESP32 creates a JSON payload containing the sensor measurements and warning code.

```cpp
snprintf(
    payload,
    sizeof(payload),
    "{\"Temperature\":%.2f,\"Humidity\":%.2f,\"Air Pressure\":%.2f,\"Warning\":%d}",
    temperature,
    humidity,
    pressure,
    warning
);
```

The payload is then published through MQTT.

```cpp
mqtt.publish(
    "batch_ds",
    payload
);
```

---

## MQTT Communication

MQTT is used as the communication protocol between the edge devices and Blynk IoT Cloud.

Secure MQTT communication is established through port:

```text
8883
```

The Raspberry Pi uses:

```text
Paho MQTT
```

The ESP32 uses:

```text
PubSubClient
```

This allows different edge-device platforms to communicate with the same centralized cloud architecture.

---

## Installation

### Raspberry Pi

Install the required Python MQTT library:

```bash
pip install paho-mqtt
```

Install Sense HAT support:

```bash
sudo apt update
sudo apt install sense-hat
```

Run Factory 1:

```bash
python3 Factory1.py
```

Run Factory 2:

```bash
python3 Factory2.py
```

---

## ESP32 Setup

Install the following libraries using the Arduino IDE Library Manager:

- PubSubClient
- DHT Sensor Library

The project also uses:

- WiFi
- WiFiClientSecure

Connect the DHT11 data pin according to the code configuration.

```cpp
#define DHT_PIN 4
```

Upload the following program to the ESP32:

```text
Factory3.ino
```

---

## Configuration

Before running the programs, configure the appropriate Blynk credentials.

### Raspberry Pi

```python
BLYNK_BROKER = "blynk.cloud"
BLYNK_PORT = 8883

BLYNK_AUTH_TOKEN = "YOUR_BLYNK_AUTH_TOKEN"
MQTT_USERNAME = "device"
```

### ESP32

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_SERVER = "YOUR_BLYNK_SERVER";
const int MQTT_PORT = 8883;

const char* MQTT_USERNAME = "device";
const char* BLYNK_AUTH_TOKEN = "YOUR_BLYNK_AUTH_TOKEN";
```

> **Security Notice:** Never upload real Wi-Fi passwords or Blynk authentication tokens to a public GitHub repository. Replace all credentials with placeholders before committing the source files.

---

## System Operation

When the system starts:

1. Each edge device initializes its sensors.
2. The device connects to the network.
3. An MQTT connection is established with Blynk Cloud.
4. Sensor measurements are collected.
5. Measurements are compared against warning thresholds.
6. A warning code is generated.
7. Sensor values and warning status are packaged into JSON.
8. The payload is transmitted to Blynk.
9. The dashboard displays the latest measurements.
10. The process repeats approximately every three seconds.

---

## Example Normal Condition

```text
Temperature  : 27.31 C
Humidity     : 42.50 %
Air Pressure : 978.24 hPa
Warning      : 0

STATUS: NORMAL
All sensor values are within limits.
```

---

## Example Warning Condition

If the temperature exceeds 35 °C:

```text
Temperature  : 36.42 C
Humidity     : 44.20 %
Air Pressure : 978.55 hPa
Warning      : 1

STATUS: WARNING
HIGH TEMPERATURE
```

The warning value is also transmitted to Blynk so that the abnormal condition can be displayed on the monitoring dashboard.

---

## Key Features

- Three simulated manufacturing shop floors
- Multi-site IIoT monitoring
- Raspberry Pi and ESP32 edge devices
- Real-time environmental sensor acquisition
- Temperature monitoring
- Humidity monitoring
- Pressure monitoring
- MQTT-based communication
- Secure MQTT/TLS connection
- Blynk IoT Cloud integration
- Centralized web/mobile monitoring
- Threshold-based warning detection
- Near-real-time data updates
- Scalable distributed architecture

---

## Future Improvements

The system can be expanded by adding:

- Real pressure sensor to the ESP32 location
- Industrial vibration sensors
- Machine operating-state monitoring
- Energy-consumption monitoring
- Historical data analytics
- Predictive maintenance
- Machine-learning-based anomaly detection
- Database integration
- Additional manufacturing locations
- Advanced alerts and notifications
- OPC UA or MTConnect integration

---

## Conclusion

This project demonstrates a practical **Multi-Site Industrial IoT Monitoring System** using Raspberry Pi, ESP32, MQTT, and Blynk IoT Cloud.

Three distributed manufacturing locations independently acquire environmental data and transmit it to a centralized cloud platform. The system provides near-real-time visibility of temperature, humidity, and pressure while automatically identifying abnormal operating conditions using threshold-based warning logic.

The prototype demonstrates how heterogeneous edge devices can be integrated into a common IIoT architecture and provides a foundation for larger industrial monitoring, analytics, and predictive-maintenance applications.

---

## Team

**Aman Harmeetsingh Bagga**  
**Ayush Katiya**

**Course:** MFG 598 – Industrial IoT  
**Instructor:** Farhad Ameri  

**Authentication Codes for trial use of the current code**
Factory 1: lNQ19bRtb0rCQEKgzYllkz_haPWCemvz  
Factory 2: t77lm7USFI14qUJHxw9a7CtoEI_L-Eu1  
Factory 3: Mp0luz5EOPMfVsyfp7-Sx3itYx25Ay3S  
