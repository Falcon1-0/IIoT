import json
import time
import ssl

import paho.mqtt.client as mqtt
from sense_hat import SenseHat


# ============================================================
# BLYNK CONFIGURATION
# ============================================================

BLYNK_BROKER = "blynk.cloud"
BLYNK_PORT = 8883

# Replace this with your Blynk device Auth Token
BLYNK_AUTH_TOKEN = ""

MQTT_USERNAME = "device"

# Send data to Blynk every 3 seconds
PUBLISH_INTERVAL = 3


# ============================================================
# WARNING THRESHOLDS
# ============================================================

# Change these values to your required limits

TEMP_LIMIT = 35.0          # Celsius
HUMIDITY_LIMIT = 50.0      # Percent
PRESSURE_LIMIT = 1020.0    # hPa


# ============================================================
# WARNING CODES FOR V6
# ============================================================

# Warning datastream:
# Name: Warning
# Virtual Pin: V6
# Type: Integer
#
# 0 = Normal
# 1 = High Temperature
# 2 = High Humidity
# 3 = High Air Pressure


# ============================================================
# INITIALIZE SENSE HAT
# ============================================================

sense = SenseHat()


# ============================================================
# INITIALIZE MQTT CLIENT
# ============================================================

client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="sensehat_raspberry_pi",
    protocol=mqtt.MQTTv311
)

client.username_pw_set(
    MQTT_USERNAME,
    BLYNK_AUTH_TOKEN
)

# Enable secure TLS connection
client.tls_set(cert_reqs=ssl.CERT_REQUIRED)


# ============================================================
# CONNECT TO BLYNK CLOUD
# ============================================================

print("Connecting to Blynk Cloud...")

client.connect(
    BLYNK_BROKER,
    BLYNK_PORT,
    keepalive=60
)

client.loop_start()

print("Blynk MQTT connection started.")
print()

print("Warning Thresholds:")
print(f"Temperature : {TEMP_LIMIT} C")
print(f"Humidity    : {HUMIDITY_LIMIT} %")
print(f"Pressure    : {PRESSURE_LIMIT} hPa")
print()


# ============================================================
# MAIN PROGRAM
# ============================================================

try:

    while True:

        # ----------------------------------------------------
        # READ SENSE HAT SENSORS
        # ----------------------------------------------------

        temperature = sense.get_temperature_from_humidity()
        humidity = sense.get_humidity()
        pressure = sense.get_pressure()


        # ----------------------------------------------------
        # DETERMINE WARNING CODE
        # ----------------------------------------------------

        # Default condition = NORMAL
        warning = 0

        # Priority 1: Temperature
        if temperature > TEMP_LIMIT:

            warning = 1

        # Priority 2: Humidity
        elif humidity > HUMIDITY_LIMIT:

            warning = 2

        # Priority 3: Air Pressure
        elif pressure > PRESSURE_LIMIT:

            warning = 3


        # ----------------------------------------------------
        # CREATE BLYNK PAYLOAD
        # ----------------------------------------------------

        # These names must match the Blynk datastream names:
        #
        # Temperature  -> V0
        # Humidity     -> V1
        # Air Pressure -> V2
        # Warning      -> V6

        payload = {

            "Temperature": round(temperature, 2),

            "Humidity": round(humidity, 2),

            "Air Pressure": round(pressure, 2),

            "Warning": warning
        }


        # ----------------------------------------------------
        # SEND DATA TO BLYNK
        # ----------------------------------------------------

        message = client.publish(
            "batch_ds",
            json.dumps(payload),
            qos=1
        )

        message.wait_for_publish(timeout=10)


        # ----------------------------------------------------
        # DISPLAY RESULTS IN TERMINAL
        # ----------------------------------------------------

        if message.is_published():

            print("=" * 55)

            print("Data sent successfully to Blynk")

            print(f"Temperature  : {temperature:.2f} C")
            print(f"Humidity     : {humidity:.2f} %")
            print(f"Air Pressure : {pressure:.2f} hPa")

            print(f"Warning V6   : {warning}")


            # ------------------------------------------------
            # DISPLAY WARNING DESCRIPTION
            # ------------------------------------------------

            if warning == 0:

                print("STATUS: NORMAL")
                print("All sensor values are within limits.")


            elif warning == 1:

                print("STATUS: WARNING")

                print(
                    f"HIGH TEMPERATURE: "
                    f"{temperature:.2f} C "
                    f"(Limit = {TEMP_LIMIT:.2f} C)"
                )


            elif warning == 2:

                print("STATUS: WARNING")

                print(
                    f"HIGH HUMIDITY: "
                    f"{humidity:.2f} % "
                    f"(Limit = {HUMIDITY_LIMIT:.2f} %)"
                )


            elif warning == 3:

                print("STATUS: WARNING")

                print(
                    f"HIGH AIR PRESSURE: "
                    f"{pressure:.2f} hPa "
                    f"(Limit = {PRESSURE_LIMIT:.2f} hPa)"
                )


            print("=" * 55)
            print()


        else:

            print("WARNING: MQTT publish was not confirmed.")


        # ----------------------------------------------------
        # WAIT BEFORE NEXT SENSOR READING
        # ----------------------------------------------------

        time.sleep(PUBLISH_INTERVAL)


# ============================================================
# STOP PROGRAM WITH CTRL+C
# ============================================================

except KeyboardInterrupt:

    print()
    print("Stopping sensor monitoring...")


# ============================================================
# CLEANUP
# ============================================================

finally:

    client.loop_stop()

    client.disconnect()

    sense.clear()

    print("Disconnected from Blynk Cloud.")
