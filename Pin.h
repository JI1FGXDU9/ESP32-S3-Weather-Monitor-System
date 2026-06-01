// ============================================================
// Pin assignment
// ピン定義
// ============================================================
// センサー	     ESP32-S3
// BME280 SDA	   GPIO1
// BME280 SCL	   GPIO2
// BME280 VCC	   3.3V
// BME280 GND	   GND
// DS18B20 DATA	 GPIO4
// DS18B20 VCC	 3.3V
// DS18B20 GND	 GND

#pragma once

// DS18B20
// DS18B20 温度センサー
#define PIN_DS18B20   4

// BME280 I2C
// BME280 I
#define PIN_I2C_SDA   5
#define PIN_I2C_SCL   6

#define PIN_LED       7