
#  Smart IoT-Based Automated Irrigation System

This project presents a smart irrigation control system using **ESP32**, various sensors, and a **Node-RED** dashboard for real-time monitoring and control. The system monitors **temperature**, **humidity**, **soil moisture**, and **water tank level**, and automatically waters the plant when needed. All sensor data is encrypted and logged in **InfluxDB**.

---

##  Features

-  Automatic watering based on soil moisture
-  Real-time temperature and humidity monitoring (DHT22)
-  Soil moisture sensing (Analog)
-  Water tank level detection (Ultrasonic HC-SR04)
-  Audible buzzer alert if tank is empty
-  Dashboard with current and historical data (Node-RED + InfluxDB)
-  Sensor data encryption using XOR
-  Manual override switch for irrigation
-  MQTT communication via EMQX broker

##  Components Used

| Component            | Description                      |
|----------------------|----------------------------------|
| ESP32                | Microcontroller (Wokwi Simulator)|
| DHT22                | Temperature and humidity sensor  |
| Soil Moisture Sensor | Analog moisture reading          |
| HC-SR04              | Ultrasonic water level detection |
| Buzzer               | Audio alert system               |
| LED (Simulated Pump) | Indicates irrigation on/off      |
| Node-RED             | Dashboard + MQTT broker interface|
| InfluxDB             | Data storage                     |
| Wokwi                | Online hardware simulator        |

---

##  MQTT Topics

- `plant/irrigation/data` → Encrypted sensor data
- `plant/irrigation/manual` → Manual ON/OFF control
- `plant/irrigation/status` → Pump state feedback
- `plant/irrigation/alert` → Tank alert messages

---

##  How it Works

- ESP32 reads sensor values every 5 seconds
- Encrypts the JSON payload with XOR
- Publishes the data via MQTT
- Node-RED receives → decrypts → displays on dashboard
- Decision logic (auto/manual) toggles pump state
- If tank is empty (distance > 20cm), buzzer sounds and alert is sent

---

##  Simulation

The full project is simulated on [Wokwi](https://wokwi.com/):

 Plug-and-play with `diagram.json`  
 Real-time serial monitor  
 Supports MQTT via WiFi emulator  

---

Acknowledgments
	•	Node-RED
	•	InfluxDB
	•	Wokwi Simulator
	•	EMQX Broker


