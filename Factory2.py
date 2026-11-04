import json
import time
import ssl

import paho.mqtt.client as mqtt
from sense_hat import SenseHat

# ---------------- BLYNK CONFIGURATION ----------------

BLYNK_BROKER = "blynk.cloud"
BLYNK_PORT = 8883

BLYNK_AUTH_TOKEN = "t77lm7USFI14qUJHxw9a7CtoEI_L-Eu1"

MQTT_USERNAME = "device"
PUBLISH_INTERVAL = 3  # seconds

# ---------------- INITIALIZE SENSOR ------------------

sense = SenseHat()

# Use the humidity sensor's temperature reading for
# a potentially better estimate of ambient temperature.
# The Pi can warm the Sense HAT and affect temperature readings.

# ---------------- INITIALIZE MQTT --------------------

client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="sensehat_raspberry_pi",
    protocol=mqtt.MQTTv311
)

client.username_pw_set(
    MQTT_USERNAME,
    BLYNK_AUTH_TOKEN
)

# Enable TLS encryption for the MQTT connection.
client.tls_set(cert_reqs=ssl.CERT_REQUIRED)

# ---------------- CONNECT TO BLYNK -------------------

print("Connecting to Blynk Cloud...")

client.connect(BLYNK_BROKER, BLYNK_PORT, keepalive=60)
client.loop_start()

try:
    while True:
        # Read sensor values
        temperature = sense.get_temperature_from_humidity()
        humidity = sense.get_humidity()
        pressure = sense.get_pressure()

        # Prepare data using exact Blynk datastream names
        payload = {
            "Temperature": round(temperature, 2),
            "Humidity": round(humidity, 2),
            "Air Pressure": round(pressure, 2)
        }

        # Publish all three measurements in one MQTT message
        message = client.publish(
            "batch_ds",
            json.dumps(payload),
            qos=1
        )

        message.wait_for_publish(timeout=10)

        if message.is_published():
            print("Data sent successfully:")
            print(json.dumps(payload, indent=2))
        else:
            print("Warning: MQTT publish not confirmed")

        time.sleep(PUBLISH_INTERVAL)

except KeyboardInterrupt:
    print("\nStopping sensor monitoring...")

finally:
    client.loop_stop()
    client.disconnect()
    sense.clear()
