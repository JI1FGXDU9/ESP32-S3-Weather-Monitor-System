# README.md

# ESP32 Weather Monitor System Ver0.0.1

## Language

* English (this page)
* [日本語 README](README_JP.md)

## Overview

This is a weather monitoring system based on the ESP32-S3.

The system measures:

* Temperature
* Humidity
* Atmospheric Pressure

in real time, uploads the data to a web server, and displays graphs in a web browser.

This project was created to monitor the tropical climate of Ozamiz City, Mindanao Island, Philippines.

In particular, it can record in real time:

* High temperatures under direct sunlight
* High nighttime humidity
* Temperature fluctuations caused by changing weather conditions

---

# System Configuration

## ESP32_Weather_Monitor_Ver0.0.1.ino

This is the main program running on the ESP32-S3.

### Main Features

* BME280 temperature, humidity, and pressure sensor reading
* DS18B20 temperature sensor reading
* WiFi connection
* HTTP POST data upload
* JSON data generation
* Static IP support
* Serial configuration interface
* EEPROM settings storage

### Sensors Used

| Sensor  | Purpose                                    |
| ------- | ------------------------------------------ |
| BME280  | Temperature, Humidity, Pressure            |
| DS18B20 | Temperature measurement in direct sunlight |

### Measurement Items

* Shade temperature
* Sunlight temperature
* Humidity
* Atmospheric pressure

---

# Weather_Monitor_Settings.py

This is a Windows GUI tool used to configure the ESP32.

### Main Features

* COM port selection
* Load settings from ESP32
* Save settings to ESP32
* WiFi configuration
* Upload URL configuration
* Static IP configuration

### Required Library

pyserial

Installation:

```bash
python -m pip install pyserial
```

### Build EXE

```bash
pyinstaller --onefile --windowed --icon=weather_settings_icon.ico Weather_Monitor_Settings.py
```

---

# Web_Weather/upload.php

This PHP program receives data uploaded from the ESP32.

### Main Features

* HTTP POST reception from ESP32
* JSON data storage
* CSV history logging
* Upload Key authentication
* Latest data update

### Stored Files

| File        | Description             |
| ----------- | ----------------------- |
| data.json   | Latest weather data     |
| history.csv | Historical weather data |

---

# Web_Weather/index.html

This web page displays weather data in a browser.

### Main Features

* Real-time graph display
* Sunlight and shade temperature display
* Humidity display
* Pressure display
* Chart.js support
* Smartphone compatible
* Date switching display
* 24-hour graph display

### Graph Contents

| Graph       | Description          |
| ----------- | -------------------- |
| Red Line    | Sunlight temperature |
| Blue Line   | Shade temperature    |
| Green Line  | Humidity             |
| Yellow Line | Atmospheric pressure |

---

# Operating Environment

## ESP32 Side

* ESP32-S3
* Arduino IDE
* ESP32 Board Package

### Required Libraries

* Adafruit BME280
* Adafruit Unified Sensor
* OneWire
* DallasTemperature

---

# Server Side

* PHP 7 or later
* Web server (Apache, etc.)

---

# Features

This system is more than just a thermometer.

It can monitor environmental changes unique to tropical regions in real time, including:

* Solar radiation
* Radiated heat
* Humidity
* Tropical nights

In the Philippines, for example:

* Nighttime humidity often exceeds 80%
* Temperatures under direct sunlight can exceed 50°C

making it possible to collect very interesting environmental data.

---

# Live Data

[Live Weather Monitor]
 https://ji1fgx.com/weather/index.html

---

# Detailed Explanation

 https://ji1fgx.com/en/260531.html

---

# License

Free for personal use and modification.

Use at your own risk.

---

Kouichi Ueno
JI1FGX/DU9
[du9@ji1fgx.com](mailto:du9@ji1fgx.com)
