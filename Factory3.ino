
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// ---------------- WI-FI SETTINGS ----------------
const char* WIFI_SSID = "GatewayTempe_WiFi";
const char* WIFI_PASSWORD = "MCG0449719rt";

// ---------------- BLYNK MQTT SETTINGS ----------------
// Replace with your assigned regional hostname.
const char* MQTT_SERVER = "ny3.blynk.cloud";
const int MQTT_PORT = 8883;

const char* MQTT_USER = "device";
const char* BLYNK_TOKEN = "Mp0luz5EOPMfVsyfp7-Sx3itYx25Ay3S";

// ---------------- DHT11 SETTINGS ----------------
#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// ---------------- MQTT OBJECTS ----------------
WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

// ---------------- TIMING ----------------
unsigned long lastPublish = 0;
const unsigned long publishInterval = 3000;

// ---------------- WIFI CONNECTION ----------------
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ---------------- MQTT CONNECTION ----------------
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to Blynk MQTT...");

    String clientId = "ESP32-";
    clientId += WiFi.macAddress();
    clientId.replace(":", "");

    if (mqtt.connect(
          clientId.c_str(),
          MQTT_USER,
          BLYNK_TOKEN)) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, state = ");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

// ---------------- SEND SENSOR DATA ----------------
void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Simulated pressure: 970.00 to 980.00 hPa
  float pressure = random(97000, 98001) / 100.0f;

  // Check actual DHT11 readings
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 reading failed!");
    return;
  }

  char payload[160];

  snprintf(
    payload,
    sizeof(payload),
    "{\"Temperature\":%.2f,\"Humidity\":%.2f,\"Air Pressure\":%.2f}",
    temperature,
    humidity,
    pressure
  );

  bool success = mqtt.publish("batch_ds", payload);

  Serial.println("------------------------------");
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity, 2);
  Serial.println(" %");

  Serial.print("Simulated pressure: ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");

  if (success) {
    Serial.print("MQTT published: ");
    Serial.println(payload);
  } else {
    Serial.println("MQTT publish failed!");
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize DHT11
  dht.begin();

  // Seed random number generator
  randomSeed(esp_random());

  // Connect to Wi-Fi
  connectWiFi();

  // TLS setup for testing
  // For production, use secureClient.setCACert()
  // with the appropriate trusted root CA certificate.
  secureClient.setInsecure();

  // Configure MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setBufferSize(256);
  mqtt.setKeepAlive(45);

  Serial.println("ESP32 DHT11 MQTT monitor started.");
}

// ---------------- MAIN LOOP ----------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();

  if (millis() - lastPublish >= publishInterval) {
    lastPublish = millis();
    sendSensorData();
  }
}
