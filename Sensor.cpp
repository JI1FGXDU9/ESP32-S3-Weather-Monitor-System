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

  if (sensorMode == SENSOR_NONE) {
    return true;
  }

  if (sensorMode & SENSOR_BME280) {

    Wire.begin(
      PIN_I2C_SDA,
      PIN_I2C_SCL
    );

    if (!bme.begin(0x76)) {
      return false;
    }
  }

  if (sensorMode & SENSOR_DS18B20) {
    ds18.begin();
  }

  return true;
}

// ============================================================
// Read sensors
// センサー読み取り
// ------------------------------------------------------------
// Reads enabled sensors according to sensorMode.
//
// sensorMode:
//
//   SENSOR_NONE
//     No sensor reading
//     センサー読み取りなし
//
//   SENSOR_BME280
//     Read BME280 only
//     BME280のみ読み取り
//
//   SENSOR_DS18B20
//     Read DS18B20 only
//     DS18B20のみ読み取り
//
//   SENSOR_BOTH
//     Read both BME280 and DS18B20
//     BME280とDS18B20の両方を読み取り
//
// Updated values:
//
//   shadeTemp
//     BME280 temperature
//     BME280温度
//
//   humidity
//     BME280 humidity
//     BME280湿度
//
//   pressure
//     BME280 pressure (hPa)
//     BME280気圧(hPa)
//
//   sunTemp
//     DS18B20 temperature
//     DS18B20温度
// ============================================================

bool readSensors() {

  // ==========================================================
  // BME280
  // ==========================================================
  if (sensorMode & SENSOR_BME280) {

    shadeTemp = bme.readTemperature();
    humidity  = bme.readHumidity();
    pressure  = bme.readPressure() / 100.0;
  }

  // ==========================================================
  // DS18B20
  // ==========================================================
  if (sensorMode & SENSOR_DS18B20) {

    ds18.requestTemperatures();

    sunTemp = ds18.getTempCByIndex(0);

  } else {

    // DS18B20 disabled
    // DS18B20無効
    sunTemp = -127.0;
  }

  return true;
}