#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 14
#define DHTTYPE DHT22
#define SOIL_PIN 32
#define PUMP_PIN 2
#define BUZZER_PIN 17
//define sound speed in cm/uS
#define SOUND_SPEED 0.034

long duration;
float distanceCm;

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";

const int trigPin = 25;
const int echoPin = 33;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

bool manualOverride = false;
bool lastPumpState = false;
const byte xorKey = 0x5A;

// WiFi Setup
void setup_wifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());
}

// XOR Encryption
String xorEncrypt(const String& input) {
  String output = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    output += char(input[i] ^ xorKey);
  }
  return output;
}

// Ultrasonic Distance
long readDistanceCM() { 
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  
  delay(1000);
  return duration == 0 ? 999 : distanceCm;
}

// Send pump status to Node-RED
void sendPumpStatus(bool state) {
  String status = state ? "ON" : "OFF";
  client.publish("plant/irrigation/status", status.c_str());
  Serial.print("Pump status sent: ");
  Serial.println(status);
}

// Send alert to Node-RED if tank empty
void sendTankAlert() {
  client.publish("plant/irrigation/alert", "Water tank empty!");
  Serial.println("Tank alert sent!");
}

void sendTankAlertOkay() {
  client.publish("plant/irrigation/alert", "Water tank okay!");
  Serial.println("Tank alert sent!");
}

// MQTT Callback
void callback(char* topic, byte* payload, unsigned int length) {
  String strPayload = "";
  for (int i = 0; i < length; i++) strPayload += (char)payload[i];

  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, strPayload)) return;

  if (String(topic) == "plant/irrigation/manual") {
    String sw = doc["switch"];
    if (sw == "ON") {
      manualOverride = true;
      digitalWrite(PUMP_PIN, HIGH);
      sendPumpStatus(true);
    } else if (sw == "OFF") {
      manualOverride = false;
      digitalWrite(PUMP_PIN, LOW);
      sendPumpStatus(false);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe("plant/irrigation/manual");
      Serial.println("MQTT connected and subscribed.");
    } else {
      Serial.print("MQTT failed. Code=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_PIN);
  long distance = readDistanceCM();
  

  Serial.printf("Temp: %.2f°C | Hum: %.2f%% | Soil: %d | Distance: %ldcm\n", temp, hum, soil, distance);

  // ---- AUTO MODE LOGIC ----
  if (!manualOverride) {
    bool shouldIrrigate = soil < 500;
    if (shouldIrrigate != lastPumpState) {
      digitalWrite(PUMP_PIN, shouldIrrigate ? HIGH : LOW);
      sendPumpStatus(shouldIrrigate);
      lastPumpState = shouldIrrigate;
    }


    static bool tankEmptyAlertSent = false;
    static bool tankOkayAlertSent = false;

    if (distance > 20 && !tankEmptyAlertSent) {
      digitalWrite(BUZZER_PIN, HIGH);
      sendTankAlert();
      tankEmptyAlertSent = true;
      tankOkayAlertSent = false;
    } else if (distance <= 20 && !tankOkayAlertSent) {
      digitalWrite(BUZZER_PIN, LOW);
      sendTankAlertOkay();
      tankOkayAlertSent = true;
      tankEmptyAlertSent = false;
    }

  } else {
    digitalWrite(BUZZER_PIN, LOW);  // silence buzzer in manual mode
  }

  // ---- ENCRYPTED DATA TO NODE-RED ----
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    String payload = "{\"temperature\":" + String(temp) +
                     ",\"humidity\":" + String(hum) +
                     ",\"soil\":" + String(soil) +
                     ",\"tankLevel\":" + String(distance) + "}";

    String encrypted = xorEncrypt(payload);
    client.publish("plant/irrigation/data", encrypted.c_str());
    Serial.println("Encrypted sensor data sent.");
  }

  delay(100);
}