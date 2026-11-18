#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// ============================================================
// WI-FI CONFIGURATION
// ============================================================

const char* WIFI_SSID = "GatewayTempe_WiFi";
const char* WIFI_PASSWORD = "MCG0449719rt";


// ============================================================
// BLYNK MQTT CONFIGURATION
// ============================================================

const char* MQTT_SERVER = "ny3.blynk.cloud";
const int MQTT_PORT = 8883;

const char* MQTT_USERNAME = "device";
const char* BLYNK_AUTH_TOKEN = "";

// Send data to Blynk every 3 seconds
const unsigned long PUBLISH_INTERVAL = 3000;


// ============================================================
// WARNING THRESHOLDS
// ============================================================

const float TEMP_LIMIT = 35.0;       // Celsius
const float HUMIDITY_LIMIT = 50.0;   // Percent
const float PRESSURE_LIMIT = 1020.0; // hPa


// ============================================================
// WARNING CODES FOR V6
// ============================================================

// Warning datastream:
// Name: Warning
// Virtual Pin: V6
// Type: Integer
//
// 0 = Normal
// 1 = High Temperature
// 2 = High Humidity
// 3 = High Air Pressure


// ============================================================
// DHT11 CONFIGURATION
// ============================================================

#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);


// ============================================================
// INITIALIZE MQTT CLIENT
// ============================================================

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);


// ============================================================
// TIMING
// ============================================================

unsigned long lastPublish = 0;


// ============================================================
// CONNECT TO WI-FI
// ============================================================

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

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
}


// ============================================================
// CONNECT TO BLYNK MQTT
// ============================================================

void connectMQTT() {

  while (!mqtt.connected()) {

    Serial.print("Connecting to Blynk Cloud...");

    String clientId = "ESP32-";
    clientId += WiFi.macAddress();
    clientId.replace(":", "");

    if (mqtt.connect(
          clientId.c_str(),
          MQTT_USERNAME,
          BLYNK_AUTH_TOKEN)) {

      Serial.println("Connected!");
      Serial.println();

    } else {

      Serial.print("Failed, MQTT state = ");
      Serial.println(mqtt.state());

      delay(3000);
    }
  }
}


// ============================================================
// READ SENSORS AND SEND DATA
// ============================================================

void sendSensorData() {

  // ----------------------------------------------------------
  // READ DHT11 SENSORS
  // ----------------------------------------------------------

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Simulated air pressure: 970.00 to 980.00 hPa
  float pressure = random(97000, 98001) / 100.0f;


  // ----------------------------------------------------------
  // CHECK SENSOR READINGS
  // ----------------------------------------------------------

  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("WARNING: DHT11 reading failed!");
    return;
  }


  // ----------------------------------------------------------
  // DETERMINE WARNING CODE
  // ----------------------------------------------------------

  // Default condition = NORMAL
  int warning = 0;

  // Priority 1: Temperature
  if (temperature > TEMP_LIMIT) {

    warning = 1;
  }

  // Priority 2: Humidity
  else if (humidity > HUMIDITY_LIMIT) {

    warning = 2;
  }

  // Priority 3: Air Pressure
  else if (pressure > PRESSURE_LIMIT) {

    warning = 3;
  }


  // ----------------------------------------------------------
  // CREATE BLYNK PAYLOAD
  // ----------------------------------------------------------

  // These names must match the Blynk datastream names:
  //
  // Temperature  -> V0
  // Humidity     -> V1
  // Air Pressure -> V2
  // Warning      -> V6

  char payload[200];

  snprintf(
    payload,
    sizeof(payload),
    "{\"Temperature\":%.2f,\"Humidity\":%.2f,\"Air Pressure\":%.2f,\"Warning\":%d}",
    temperature,
    humidity,
    pressure,
    warning
  );


  // ----------------------------------------------------------
  // SEND DATA TO BLYNK
  // ----------------------------------------------------------

  bool success = mqtt.publish(
    "batch_ds",
    payload
  );


  // ----------------------------------------------------------
  // DISPLAY RESULTS IN SERIAL MONITOR
  // ----------------------------------------------------------

  Serial.println("=======================================================");

  if (success) {

    Serial.println("Data sent successfully to Blynk");
  }

  else {

    Serial.println("WARNING: MQTT publish failed!");
  }

  Serial.println();

  Serial.print("Temperature  : ");
  Serial.print(temperature, 2);
  Serial.println(" C");

  Serial.print("Humidity     : ");
  Serial.print(humidity, 2);
  Serial.println(" %");

  Serial.print("Air Pressure : ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");

  Serial.print("Warning V6   : ");
  Serial.println(warning);

  Serial.println();


  // ----------------------------------------------------------
  // DISPLAY WARNING DESCRIPTION
  // ----------------------------------------------------------

  if (warning == 0) {

    Serial.println("STATUS: NORMAL");
    Serial.println("All sensor values are within limits.");
  }

  else if (warning == 1) {

    Serial.println("STATUS: WARNING");

    Serial.print("HIGH TEMPERATURE: ");
    Serial.print(temperature, 2);
    Serial.print(" C (Limit = ");
    Serial.print(TEMP_LIMIT, 2);
    Serial.println(" C)");
  }

  else if (warning == 2) {

    Serial.println("STATUS: WARNING");

    Serial.print("HIGH HUMIDITY: ");
    Serial.print(humidity, 2);
    Serial.print(" % (Limit = ");
    Serial.print(HUMIDITY_LIMIT, 2);
    Serial.println(" %)");
  }

  else if (warning == 3) {

    Serial.println("STATUS: WARNING");

    Serial.print("HIGH AIR PRESSURE: ");
    Serial.print(pressure, 2);
    Serial.print(" hPa (Limit = ");
    Serial.print(PRESSURE_LIMIT, 2);
    Serial.println(" hPa)");
  }

  Serial.println("=======================================================");
  Serial.println();
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // ----------------------------------------------------------
  // INITIALIZE DHT11
  // ----------------------------------------------------------

  dht.begin();


  // ----------------------------------------------------------
  // INITIALIZE RANDOM PRESSURE GENERATOR
  // ----------------------------------------------------------

  randomSeed(esp_random());


  // ----------------------------------------------------------
  // CONNECT TO WI-FI
  // ----------------------------------------------------------

  connectWiFi();


  // ----------------------------------------------------------
  // TLS CONFIGURATION
  // ----------------------------------------------------------

  // TLS encryption enabled without certificate verification.
  // For production, use setCACert() with the appropriate
  // trusted root CA certificate.

  secureClient.setInsecure();


  // ----------------------------------------------------------
  // CONFIGURE MQTT
  // ----------------------------------------------------------

  mqtt.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );

  mqtt.setBufferSize(256);

  mqtt.setKeepAlive(60);


  // ----------------------------------------------------------
  // STARTUP INFORMATION
  // ----------------------------------------------------------

  Serial.println("ESP32 DHT11 Blynk MQTT monitor started.");
  Serial.println();

  Serial.println("Warning Thresholds:");

  Serial.print("Temperature : ");
  Serial.print(TEMP_LIMIT);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(HUMIDITY_LIMIT);
  Serial.println(" %");

  Serial.print("Pressure    : ");
  Serial.print(PRESSURE_LIMIT);
  Serial.println(" hPa");

  Serial.println();
}


// ============================================================
// MAIN PROGRAM
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // CHECK WI-FI CONNECTION
  // ----------------------------------------------------------

  if (WiFi.status() != WL_CONNECTED) {

    connectWiFi();
  }


  // ----------------------------------------------------------
  // CHECK MQTT CONNECTION
  // ----------------------------------------------------------

  if (!mqtt.connected()) {

    connectMQTT();
  }


  // ----------------------------------------------------------
  // PROCESS MQTT
  // ----------------------------------------------------------

  mqtt.loop();


  // ----------------------------------------------------------
  // SEND SENSOR DATA EVERY 3 SECONDS
  // ----------------------------------------------------------

  if (millis() - lastPublish >= PUBLISH_INTERVAL) {

    lastPublish = millis();

    sendSensorData();
  }
}
