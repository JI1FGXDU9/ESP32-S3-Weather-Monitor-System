#include <Wire.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Pin.h"
#include "Sensor.h"

// ============================================================
// BME280
// ============================================================

Adafruit_BME280 bme;

// ============================================================
// DS18B20
// ============================================================

OneWire oneWire(PIN_DS18B20);

DallasTemperature ds18(&oneWire);

// ============================================================
// Sensor values
// ============================================================

float shadeTemp = 0;
float sunTemp   = 0;
float humidity  = 0;
float pressure  = 0;

// ============================================================
// Initialize sensors
// センサー初期化
// ============================================================

bool initSensors() {

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!bme.begin(0x76)) {
    return false;
  }

  ds18.begin();

  return true;
}

// ============================================================
// Read sensors
// センサー読み取り
// ============================================================

bool readSensors() {
///  return true;
  // BME280
  shadeTemp = bme.readTemperature();

  humidity = bme.readHumidity();

  pressure = bme.readPressure() / 100.0;


  // DS18B20
  ds18.requestTemperatures();

  sunTemp = ds18.getTempCByIndex(0);

  return true;
}