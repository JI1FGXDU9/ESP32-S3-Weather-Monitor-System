#pragma once

// ============================================================
// Sensor mode definitions
// センサーモード定義
// ============================================================

#define SENSOR_NONE      0
#define SENSOR_BME280    1
#define SENSOR_DS18B20   2
#define SENSOR_BOTH      3

// ============================================================
// Default sensor mode
// デフォルトセンサーモード
// ============================================================

#define DEFAULT_SENSOR_MODE SENSOR_BOTH

// ============================================================
// Default network settings
// デフォルトネットワーク設定
// ============================================================

#define DEFAULT_SSID       ""
#define DEFAULT_PASS       ""

#define DEFAULT_UPLOAD_URL \
  "http://example.com/weather/upload.php"

#define DEFAULT_UPLOAD_KEY "test123"

// ============================================================
// Default IP settings
// デフォルトIP設定
// ============================================================

#define DEFAULT_LOCAL_IP   "192.168.1.251"
#define DEFAULT_GATEWAY    "192.168.1.1"
#define DEFAULT_SUBNET     "255.255.255.0"