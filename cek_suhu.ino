#define BLYNK_TEMPLATE_ID "TMPL6Em2dbOi9"
#define BLYNK_TEMPLATE_NAME "Sensor Tubuh"
#define BLYNK_AUTH_TOKEN "urJC9SH1WyEXhGojChm9NGENHEAAY2Jq"


#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// WiFi credentials
char ssid[] = "Realme";
char pass[] = "rmu11345";

// MAX30100
PulseOximeter pox;
uint32_t tsLastReport = 0;
#define REPORTING_PERIOD_MS 1000

// DS18B20 (body temp in °F)
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature bodyTempSensor(&oneWire);

// DHT11 (room temp in °C)
#define DHTPIN 16
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Blynk timer
BlynkTimer timer;

// Beat detected callback
void onBeatDetected() {
  Serial.println("Beat!");
}

// Function to read DS18B20 and DHT11
void readOtherSensors() {
  // DS18B20
  bodyTempSensor.requestTemperatures();
  float tempC = bodyTempSensor.getTempCByIndex(0);
  float tempF = tempC * 1.8 + 32.0;

  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print("Body Temp (°F): ");
    Serial.println(tempF);
    Blynk.virtualWrite(V2, tempF);
  } else {
    Serial.println("Error: DS18B20 not responding");
  }

  // DHT11
  float roomTemp = dht.readTemperature();
  if (!isnan(roomTemp)) {
    Serial.print("Room Temp (°C): ");
    Serial.println(roomTemp);
    Blynk.virtualWrite(V3, roomTemp);
  } else {
    Serial.println("Error: DHT11 read failed");
  }
}

void setup() {
  Serial.begin(115200);

  // Start Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Start sensors
  dht.begin();
  bodyTempSensor.begin();

  // Start MAX30100
  if (!pox.begin()) {
    Serial.println("MAX30100 INIT FAILED");
    while (1);  // Stop here if it fails
  } else {
    Serial.println("MAX30100 INIT SUCCESS");
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Set periodic sensor read every 3 seconds
  timer.setInterval(3000L, readOtherSensors);
}

void loop() {
  Blynk.run();
  timer.run();
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    tsLastReport = millis();

    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    Serial.print("Heart Rate: ");
    Serial.print(hr);
    Serial.print(" bpm | SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");

    Blynk.virtualWrite(V0, hr);
    Blynk.virtualWrite(V1, spo2);
  }
}