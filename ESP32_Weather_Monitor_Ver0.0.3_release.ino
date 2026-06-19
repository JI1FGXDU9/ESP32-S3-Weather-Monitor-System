// ============================================================
// Additional library installation required
// ライブラリーの追加インストールが必要
// ------------------------------------------------------------
//
// Arduino IDE:
// Arduino IDE:
//
// Sketch
//  ↓
// Include Library
//  ↓
// Manage Libraries
//
// スケッチ
//  ↓
// ライブラリをインクルード
//  ↓
// ライブラリを管理
//
// Required libraries:
// 必要ライブラリー:
//
// - Adafruit BME280 Library
// - Adafruit Unified Sensor
// - DallasTemperature
// - OneWire
//
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "Pin.h"
#include "Sensor.h"

#include <Preferences.h>
#include "Config.h"
#include "localcard.h"

int sensorMode = DEFAULT_SENSOR_MODE;

// ============================================================
// Preferences library
// Preferencesライブラリー
// ------------------------------------------------------------
// Used to save configuration data into ESP32 NVS flash memory
//
// ESP32のNVSフラッシュメモリーへ
// 設定データを保存するために使用
// ============================================================
#include <Preferences.h>


// ============================================================
// Preferences object
// Preferencesオブジェクト
// ------------------------------------------------------------
// Used for reading/writing configuration values
//
// 設定値の読み書きに使用
// ============================================================
Preferences prefs;


// ============================================================
// WiFi configuration variables
// WiFi設定変数
// ------------------------------------------------------------
// These values are loaded from Preferences at startup
//
// 起動時に Preferences から読み込まれる
// ============================================================

// WiFi SSID
// WiFi SSID
String cfg_ssid;

// WiFi password
// WiFiパスワード
String cfg_pass;


// ============================================================
// Upload server configuration
// アップロードサーバー設定
// ============================================================

// Upload destination URL
// アップロード先URL
String cfg_uploadUrl;

// Upload authentication key
// アップロード認証キー
String cfg_uploadKey;


// ============================================================
// Network configuration
// ネットワーク設定
// ------------------------------------------------------------
// Static IP address settings
//
// 固定IPアドレス設定
// ============================================================

// ESP32 local IP address
// ESP32 ローカルIPアドレス
IPAddress localIP;

// Gateway address
// ゲートウェイアドレス
IPAddress gateway;

// Subnet mask
// サブネットマスク
IPAddress subnet;

// ============================================================
// Debug serial output control
// デバッグ用シリアル出力制御
// ------------------------------------------------------------
// DEBUG_SERIAL = 1
//   Enable debug messages to Serial monitor
//   シリアルモニタへのデバッグ出力を有効
//
// DEBUG_SERIAL = 0
//   Disable all debug messages
//   すべてのデバッグ出力を無効
// ============================================================

#define DEBUG_SERIAL 0

#if DEBUG_SERIAL

  // ==========================================================
  // Debug output enabled
  // デバッグ出力 有効
  // ----------------------------------------------------------
  // DBG_PRINT()   -> Serial.print()
  // DBG_PRINTLN() -> Serial.println()
  // ==========================================================

  #define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)

#else

  // ==========================================================
  // Debug output disabled
  // デバッグ出力 無効
  // ----------------------------------------------------------
  // Macros expand to nothing
  // マクロは何も出力しない
  // ==========================================================

  #define DBG_PRINT(...)
  #define DBG_PRINTLN(...)

#endif

// ============================================================
// DNS server settings
// DNSサーバー設定
// ------------------------------------------------------------
// Fixed DNS servers
// DNSサーバーは固定
// ============================================================

// Google DNS
// Google DNS
IPAddress dns1(8, 8, 8, 8);

// Cloudflare DNS
// Cloudflare DNS
IPAddress dns2(1, 1, 1, 1);

// ============================================================
// Web server instance
// Webサーバーインスタンス
// ------------------------------------------------------------
// Create an HTTP web server on port 80
// ポート80で動作するHTTP Webサーバーを作成
//
// Port 80 is the standard HTTP port
// 80番ポートは標準HTTPポート
// ============================================================

WebServer server(80);


void loadConfig() {

  prefs.begin("weather", false);

  cfg_ssid      = prefs.getString("ssid", DEFAULT_SSID);
  cfg_pass      = prefs.getString("pass", DEFAULT_PASS);
  cfg_uploadUrl = prefs.getString("url",  DEFAULT_UPLOAD_URL);
  cfg_uploadKey = prefs.getString("key",  DEFAULT_UPLOAD_KEY);

  localIP.fromString(prefs.getString("ip", DEFAULT_LOCAL_IP));
  gateway.fromString(prefs.getString("gw", DEFAULT_GATEWAY));
  subnet.fromString(prefs.getString("sn", DEFAULT_SUBNET));
  sensorMode =
    prefs.getInt(
      "sensor",
      DEFAULT_SENSOR_MODE
    );
}

// ============================================================
// Handle root page request
// ルートページ要求処理
// ------------------------------------------------------------
// Generates and sends the weather monitor HTML page
// 気象モニター用HTMLページを生成して送信する
//
// The page displays:
// ページには以下を表示する：
//
// - Shade temperature
//   日陰温度
//
// - Sunlight temperature
//   日向温度
//
// - Humidity
//   湿度
//
// - Atmospheric pressure
//   気圧
// ============================================================
void handleRoot() {

  // ==========================================================
  // HTML template
  // HTMLテンプレート
  // ==========================================================

  String html = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>Weather Monitor</title>

<style>

body {
  font-family: Arial;
  background:#202020;
  color:white;
  text-align:center;
}

.card {
  margin:20px auto;
  padding:20px;
  max-width:360px;
  background:#303030;
  border-radius:12px;
}

.big {
  font-size:48px;
  color:orange;
}

.item {
  font-size:22px;
  margin:12px;
}

.mode {
  font-size:18px;
  color:#ffcc66;
  margin-bottom:12px;
}

</style>

</head>

<body>

<h1>%CARD_TITLE%</h1>

<div class="card">

  <div id="mode" class="mode">%MODE%</div>

  <div>%LABEL_SHADE_TEMP%</div>
  <div id="shadeTemp" class="big">%SHADE_TEMP%</div>

  <div class="item">
    %LABEL_SUN_TEMP% :
    <span id="sunTemp">%SUN_TEMP%</span>
  </div>

  <div class="item">
    %LABEL_HUMIDITY% :
    <span id="humidity">%HUMIDITY%</span>
  </div>

  <div class="item">
    %LABEL_PRESSURE% :
    <span id="pressure">%PRESSURE%</span>
  </div>

</div>

<script>

// ==========================================================
// Update local card every 60 seconds
// ローカルカードを60秒ごとに更新
// ==========================================================

function updateCard() {

  fetch("/data")
    .then(response => response.json())
    .then(data => {

      document.getElementById("mode").textContent      = data.mode;
      document.getElementById("shadeTemp").textContent = data.shadeTemp;
      document.getElementById("sunTemp").textContent   = data.sunTemp;
      document.getElementById("humidity").textContent  = data.humidity;
      document.getElementById("pressure").textContent  = data.pressure;

    })
    .catch(error => {
      console.log("Data update error:", error);
    });
}

// First update after page load
// ページ読み込み直後に1回更新
updateCard();

// Update every 60 seconds
// 60秒ごとに更新
setInterval(updateCard, 60000);

</script>

</body>
</html>
)rawliteral";


  // ==========================================================
  // Sensor mode check
  // センサーモード判定
  // ==========================================================

  bool useBme280 =
    (sensorMode == SENSOR_BME280 ||
     sensorMode == SENSOR_BOTH);

  bool useDs18b20 =
    (sensorMode == SENSOR_DS18B20 ||
     sensorMode == SENSOR_BOTH);

  bool dummyMode =
    (sensorMode == SENSOR_NONE);


  // ==========================================================
  // Replace fixed labels
  // 固定表示ラベルを置換
  // ==========================================================

  html.replace("%CARD_TITLE%",       CARD_TITLE);
  html.replace("%LABEL_SHADE_TEMP%", LABEL_SHADE_TEMP);
  html.replace("%LABEL_SUN_TEMP%",   LABEL_SUN_TEMP);
  html.replace("%LABEL_HUMIDITY%",   LABEL_HUMIDITY);
  html.replace("%LABEL_PRESSURE%",   LABEL_PRESSURE);


  // ==========================================================
  // Replace sensor values
  // センサー値を置換
  // ==========================================================

  if (dummyMode) {

    html.replace("%MODE%", LABEL_DUMMY);

    html.replace("%SHADE_TEMP%", "25.0 °C");
    html.replace("%SUN_TEMP%",   "35.0 °C");
    html.replace("%HUMIDITY%",   "60.0 %");
    html.replace("%PRESSURE%",   "1013.0 hPa");

  } else {

    html.replace("%MODE%", "");

    if (useBme280 && humidity > 0 && pressure > 0) {
      html.replace("%SHADE_TEMP%", String(shadeTemp, 1) + " °C");
      html.replace("%HUMIDITY%",   String(humidity, 1) + " %");
      html.replace("%PRESSURE%",   String(pressure, 1) + " hPa");
    } else {
      html.replace("%SHADE_TEMP%", "--");
      html.replace("%HUMIDITY%",   "--");
      html.replace("%PRESSURE%",   "--");
    }

    if (useDs18b20 && sunTemp > -100) {
      html.replace("%SUN_TEMP%", String(sunTemp, 1) + " °C");
    } else {
      html.replace("%SUN_TEMP%", "--");
    }
  }


  // ==========================================================
  // Send HTML response to browser
  // ブラウザへHTMLレスポンス送信
  // ==========================================================

  server.send(200, "text/html", html);
}

void handleData() {

  // ==========================================================
  // Sensor mode check
  // センサーモード判定
  // ==========================================================

  bool useBme280 =
    (sensorMode == SENSOR_BME280 ||
     sensorMode == SENSOR_BOTH);

  bool useDs18b20 =
    (sensorMode == SENSOR_DS18B20 ||
     sensorMode == SENSOR_BOTH);

  bool dummyMode =
    (sensorMode == SENSOR_NONE);


  // ==========================================================
  // Create JSON response
  // JSONレスポンス作成
  // ==========================================================

  String json = "{";

  if (dummyMode) {

    json += "\"mode\":\"Dummy Data\",";
    json += "\"shadeTemp\":\"25.0 °C\",";
    json += "\"sunTemp\":\"35.0 °C\",";
    json += "\"humidity\":\"60.0 %\",";
    json += "\"pressure\":\"1013.0 hPa\"";

  } else {

    json += "\"mode\":\"\",";

    if (useBme280 && humidity > 0 && pressure > 0) {
      json += "\"shadeTemp\":\"" + String(shadeTemp, 1) + " °C\",";
      json += "\"humidity\":\""   + String(humidity, 1)  + " %\",";
      json += "\"pressure\":\""   + String(pressure, 1)  + " hPa\",";
    } else {
      json += "\"shadeTemp\":\"--\",";
      json += "\"humidity\":\"--\",";
      json += "\"pressure\":\"--\",";
    }

    if (useDs18b20 && sunTemp > -100) {
      json += "\"sunTemp\":\"" + String(sunTemp, 1) + " °C\"";
    } else {
      json += "\"sunTemp\":\"--\"";
    }
  }

  json += "}";


  // ==========================================================
  // Send JSON response
  // JSONレスポンス送信
  // ==========================================================

  server.send(200, "application/json", json);
}

// ============================================================
// Setup function
// 初期化処理
// ------------------------------------------------------------
// This function runs once when ESP32 starts
// ESP32起動時に1回だけ実行される
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(3000);
  // ==========================================================
  // LED pin setup
  // LEDピン初期化
  // ==========================================================
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  loadConfig();

  // ==========================================================
  // Sensor initialize
  // センサー初期化
  // ==========================================================

    initSensors();

  // ==========================================================
  // WiFi station mode setup
  // WiFiステーションモード設定
  // ----------------------------------------------------------
  // WIFI_STA:
  //   Connect ESP32 to an existing WiFi router
  //   ESP32を既存のWiFiルーターに接続する
  // ==========================================================
  WiFi.mode(WIFI_STA);

  // ==========================================================
  // Static IP address setup
  // 固定IPアドレス設定
  // ==========================================================
  WiFi.config(localIP, gateway, subnet, dns1, dns2);

  // ==========================================================
  // Start WiFi connection
  // WiFi接続開始
  // ==========================================================
  WiFi.begin(cfg_ssid.c_str(), cfg_pass.c_str());

  DBG_PRINT("Connecting");

  // ==========================================================
  // Wait until WiFi is connected
  // WiFi接続完了まで待機
  // ==========================================================
  unsigned long wifiStart = millis();

  while (WiFi.status() != WL_CONNECTED &&
        millis() - wifiStart < 10000) {
    delay(500);
  }

  // ==========================================================
  // Display assigned IP address
  // 割り当てられたIPアドレスを表示
  // ==========================================================
  DBG_PRINTLN();
  DBG_PRINT("IP=");
  DBG_PRINTLN(WiFi.localIP());


  // ==========================================================
  // NTP time synchronization
  // NTP時刻同期
  // ----------------------------------------------------------
  // Philippines time is UTC+8
  // フィリピン時間は UTC+8
  // ==========================================================
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;



  // ==========================================================
  // Wait until valid local time is obtained
  // 正しいローカル時刻が取得できるまで待機
  // ==========================================================
unsigned long ntpStart = millis();

  while (!getLocalTime(&timeinfo) &&
        millis() - ntpStart < 10000) {
              DBG_PRINT(".");
    delay(500);
  }


  DBG_PRINTLN("NTP time synced.");


  // ==========================================================
  // Register web page handler
  // Webページ処理関数を登録
  // ----------------------------------------------------------
  // "/" accesses the root page
  // "/" にアクセスされた時 handleRoot() を実行
  // ==========================================================
  server.on("/", handleRoot);

  // ==========================================================
  // Web server routes
  // Webサーバールート設定
  // ==========================================================

  server.on("/", handleRoot);
  server.on("/data", handleData);

  // ==========================================================
  // Start web server
  // Webサーバー開始
  // ==========================================================
  server.begin();

  DBG_PRINTLN("Web server started.");
}

// ============================================================
// Handle USB configuration commands
// USB設定コマンド処理
// ------------------------------------------------------------
// Processes configuration commands received from USB serial.
//
// USBシリアルから受信した設定コマンドを処理する。
//
// Supported commands:
// 対応コマンド:
//
// LOAD
//   Read current settings
//   現在設定読み込み
//
// SET KEY VALUE
//   Save configuration parameter
//   設定値保存
//
// Example:
// 例:
//
// SET SSID MyWiFi
// SET PASS 12345678
// ============================================================
void handleConfigCommand(String line) {

  // ==========================================================
  // LOAD command
  // LOADコマンド
  // ----------------------------------------------------------
  // Send current settings to USB serial
  // 現在設定をUSBシリアルへ送信
  // ==========================================================
  if (line == "LOAD") {

    // WiFi SSID
    // WiFi SSID
    Serial.println("SSID=" + cfg_ssid);

    // WiFi password
    // WiFiパスワード
    Serial.println("PASS=" + cfg_pass);

    // Upload URL
    // アップロードURL
    Serial.println("UPLOAD_URL=" + cfg_uploadUrl);

    // Upload authentication key
    // アップロード認証キー
    Serial.println("UPLOAD_KEY=" + cfg_uploadKey);

    // Local IP address
    // ローカルIPアドレス
    Serial.println("LOCAL_IP=" + localIP.toString());

    // Gateway address
    // ゲートウェイアドレス
    Serial.println("GATEWAY=" + gateway.toString());

    // Subnet mask
    // サブネットマスク
    Serial.println("SUBNET=" + subnet.toString());
    // --------------------------------------------------------
    // Sensor mode setting
    // センサーモード設定
    //
    // 0 = No sensor
    // 1 = BME280 only
    // 2 = DS18B20 only
    // 3 = BME280 + DS18B20
    //
    // 0 = センサーなし
    // 1 = BME280のみ
    // 2 = DS18B20のみ
    // 3 = BME280 + DS18B20
    // --------------------------------------------------------
    Serial.println("SENSOR=" + String(sensorMode));

    // End of LOAD response
    // LOAD応答終了
    Serial.println("END");

    return;
  }


  // ==========================================================
  // SET command
  // SETコマンド
  // ----------------------------------------------------------
  // Format:
  // 形式:
  //
  // SET KEY VALUE
  // ==========================================================
  if (line.startsWith("SET ")) {

    // ========================================================
    // Find separator position
    // 区切り位置検索
    // --------------------------------------------------------
    // Example:
    // 例:
    //
    // SET SSID MyWiFi
    //          ^
    // ========================================================
    int sp = line.indexOf(' ', 4);


    // ========================================================
    // Invalid command format
    // コマンド形式エラー
    // ========================================================
    if (sp < 0) {

      Serial.println("ERR");

      return;
    }


    // ========================================================
    // Extract KEY
    // KEY取得
    // ========================================================
    String key = line.substring(4, sp);


    // ========================================================
    // Extract VALUE
    // VALUE取得
    // ========================================================
    String val = line.substring(sp + 1);

    // ========================================================
    // Empty value marker
    // 空白値マーカー
    // ========================================================
    if (val == "__EMPTY__") {
      val = "";
    }
    
    // ========================================================
    // SSID setting
    // SSID設定
    // ========================================================
    if (key == "SSID") {

      // Save to Preferences
      // Preferencesへ保存
      prefs.putString("ssid", val);

      // Update current variable
      // 現在変数更新
      cfg_ssid = val;


    // ========================================================
    // Password setting
    // パスワード設定
    // ========================================================
    } else if (key == "PASS") {

      prefs.putString("pass", val);

      cfg_pass = val;


    // ========================================================
    // Upload URL setting
    // アップロードURL設定
    // ========================================================
    } else if (key == "UPLOAD_URL") {

      prefs.putString("url", val);

      cfg_uploadUrl = val;


    // ========================================================
    // Upload key setting
    // アップロードキー設定
    // ========================================================
    } else if (key == "UPLOAD_KEY") {

      prefs.putString("key", val);

      cfg_uploadKey = val;


    // ========================================================
    // Local IP setting
    // ローカルIP設定
    // ========================================================
    } else if (key == "LOCAL_IP") {

      // Save IP string
      // IP文字列保存
      prefs.putString("ip", val);

      // Convert string to IPAddress
      // 文字列をIPAddressへ変換
      localIP.fromString(val);


    // ========================================================
    // Gateway setting
    // ゲートウェイ設定
    // ========================================================
    } else if (key == "GATEWAY") {

      prefs.putString("gw", val);

      gateway.fromString(val);


    // ========================================================
    // Subnet mask setting
    // サブネットマスク設定
    // ========================================================
    } else if (key == "SUBNET") {

      prefs.putString("sn", val);

      subnet.fromString(val);

    }

    // --------------------------------------------------------
    // Set sensor mode
    // センサーモード設定
    //
    // 0 = No sensor
    // 1 = BME280 only
    // 2 = DS18B20 only
    // 3 = BME280 + DS18B20
    //
    // 0 = センサーなし
    // 1 = BME280のみ
    // 2 = DS18B20のみ
    // 3 = BME280 + DS18B20
    // --------------------------------------------------------
    else if (key == "SENSOR") {

      int mode = val.toInt();

      // Accept only valid values
      // 有効な値のみ受け付ける
      if (mode >= SENSOR_NONE &&
          mode <= SENSOR_BOTH) {

        sensorMode = mode;

        // Save to flash memory
        // フラッシュメモリへ保存
        prefs.putInt("sensor", sensorMode);
      }

    // ========================================================
    // Unknown setting key
    // 不明な設定キー
    // ========================================================
    } else {

      Serial.println("UNKNOWN");

      return;
    } 

    // ========================================================
    // Command success response
    // コマンド成功応答
    // ========================================================
    Serial.println("OK");
  }
}

// ============================================================
// Handle serial input
// シリアル入力処理
// ------------------------------------------------------------
// Receives weather data from Serial input
// シリアル入力から気象データを受信する
//
// Input format:
// 入力形式:
//
// shade,sun,humidity,pressure
//
// Example:
// 例:
//
// 28.5,35.2,72.1,1008.3
// ============================================================

void handleSerialInput() {

  // ==========================================================
  // Check if serial data is available
  // シリアルデータ有無確認
  // ==========================================================
  if (!Serial.available()) return;


  // ==========================================================
  // Read one line from serial
  // シリアルから1行読み込み
  // ----------------------------------------------------------
  // Read until newline character '\n'
  // 改行文字 '\n' まで読み込む
  // ==========================================================
  String line = Serial.readStringUntil('\n');

  // Remove spaces and CR/LF
  // 前後の空白や改行除去
  line.trim();

  // Ignore empty line
  // 空行は無視
  if (line.length() == 0) return;


  // ==========================================================
  // Find comma positions
  // カンマ位置検索
  // ==========================================================
  int p1 = line.indexOf(',');
  int p2 = line.indexOf(',', p1 + 1);
  int p3 = line.indexOf(',', p2 + 1);


  // ==========================================================
  // Validate format
  // フォーマット確認
  // ==========================================================
  if (p1 < 0 || p2 < 0 || p3 < 0) {

    DBG_PRINTLN("Format: shade,sun,humidity,pressure");

    return;
  }


  // ==========================================================
  // Extract each field
  // 各項目を切り出し
  // ==========================================================
  String s1 = line.substring(0, p1);
  String s2 = line.substring(p1 + 1, p2);
  String s3 = line.substring(p2 + 1, p3);
  String s4 = line.substring(p3 + 1);


  // ==========================================================
  // Remove spaces from each field
  // 各項目の空白除去
  // ==========================================================
  s1.trim();
  s2.trim();
  s3.trim();
  s4.trim();


  // ==========================================================
  // Convert text to floating point values
  // 文字列を浮動小数点数へ変換
  // ----------------------------------------------------------
  // Only update if field is not empty
  // 空文字列でなければ更新
  // ==========================================================
  if (s1.length() > 0) shadeTemp = s1.toFloat();

  if (s2.length() > 0) sunTemp   = s2.toFloat();

  if (s3.length() > 0) humidity  = s3.toFloat();

  if (s4.length() > 0) pressure  = s4.toFloat();


  // ==========================================================
  // Display updated values
  // 更新値表示
  // ==========================================================
  Serial.println("Weather dummy values updated:");

  Serial.print("shadeTemp = ");
  Serial.println(shadeTemp, 1);

  Serial.print("sunTemp   = ");
  Serial.println(sunTemp, 1);

  Serial.print("humidity  = ");
  Serial.println(humidity, 1);

  Serial.print("pressure  = ");
  Serial.println(pressure, 1);
}
// ============================================================
// Upload weather data to server
// 気象データをサーバーへアップロード
// ------------------------------------------------------------
// Sends current sensor values to PHP server as JSON
// 現在のセンサー値をJSON形式でPHPサーバーへ送信する
// ============================================================

void uploadWeatherData() {

  // ==========================================================
  // Check WiFi connection
  // WiFi接続確認
  // ----------------------------------------------------------
  // Skip upload if WiFi is disconnected
  // WiFi未接続の場合はアップロードしない
  // ==========================================================
  if (WiFi.status() != WL_CONNECTED) {
    DBG_PRINTLN("WiFi not connected. Skip upload.");
    return;
  }


  // ==========================================================
  // Create JSON data
  // JSONデータ作成
  // ----------------------------------------------------------
  // The key must match the PHP-side upload key
  // key は PHP側の upload key と一致させる必要がある
  // ==========================================================
  String json = "{";
  json += "\"key\":\"" + cfg_uploadKey + "\",";
  json += "\"shade_temp\":" + String(shadeTemp, 1) + ",";
  json += "\"sun_temp\":"   + String(sunTemp, 1) + ",";
  json += "\"humidity\":"   + String(humidity, 1) + ",";
  json += "\"pressure\":"   + String(pressure, 1);
  json += "}";


  // ==========================================================
  // Debug output
  // デバッグ出力
  // ==========================================================
  DBG_PRINTLN("POST weather data...");
  DBG_PRINTLN(json);

  DBG_PRINT("WiFi status = ");
  DBG_PRINTLN(WiFi.status());

  DBG_PRINT("IP = ");
  DBG_PRINTLN(WiFi.localIP());


  // ==========================================================
  // DNS test
  // DNS確認
  // ----------------------------------------------------------
  // Resolve ji1fgx.com before HTTP POST
  // HTTP送信前に ji1fgx.com の名前解決を確認
  // ==========================================================
  DBG_PRINT("DNS test ji1fgx.com = ");

  IPAddress serverIP;

  if (!WiFi.hostByName("ji1fgx.com", serverIP) || serverIP.toString() == "0.0.0.0") {
    DBG_PRINTLN("DNS FAILED");
    return;
  }

  DBG_PRINTLN(serverIP);


  // ==========================================================
  // Prepare HTTP client
  // HTTPクライアント準備
  // ==========================================================
  WiFiClient client;
  HTTPClient http;

  // Set HTTP timeout to 15 seconds
  // HTTPタイムアウトを15秒に設定
  http.setTimeout(15000);


  // ==========================================================
  // Upload URL
  // アップロード先URL
  // ==========================================================
  const char* url = cfg_uploadUrl.c_str();


  // ==========================================================
  // Start HTTP connection
  // HTTP接続開始
  // ==========================================================
  if (!http.begin(client, url)) {
    DBG_PRINTLN("HTTP begin failed.");
    return;
  }


  // ==========================================================
  // Set HTTP header
  // HTTPヘッダー設定
  // ----------------------------------------------------------
  // Send data as JSON
  // JSON形式として送信
  // ==========================================================
  http.addHeader("Content-Type", "application/json");


  // ==========================================================
  // Send HTTP POST request
  // HTTP POST送信
  // ==========================================================
  int httpCode = http.POST(json);


  // ==========================================================
  // Display HTTP result
  // HTTP結果表示
  // ==========================================================
  DBG_PRINT("HTTP code = ");
  DBG_PRINTLN(httpCode);

  DBG_PRINT("HTTP error = ");
  DBG_PRINTLN(http.errorToString(httpCode));


  // ==========================================================
  // Read server response
  // サーバー応答読み取り
  // ==========================================================
  String response = http.getString();

  DBG_PRINT("Response = ");
  DBG_PRINTLN(response);


  // ==========================================================
  // Close HTTP connection
  // HTTP接続終了
  // ==========================================================
  http.end();
}
// ============================================================
// Main loop
// メインループ
// ------------------------------------------------------------
// This function runs repeatedly after setup()
// setup() の後に繰り返し実行される
// ============================================================

void loop() {

  // ==========================================================
  // Handle web server requests
  // Webサーバー要求処理
  // ==========================================================
  server.handleClient();

  // ==========================================================
  // Sensor read
  // センサー読み取り
  // ==========================================================

    readSensors();

  // ==========================================================
  // Handle USB serial input
  // USBシリアル入力処理
  // ----------------------------------------------------------
  // There are two types of serial commands:
  // シリアル入力には2種類ある:
  //
  // 1. Configuration command
  //    設定コマンド
  //
  //    LOAD
  //    SET SSID xxxxx
  //    SET PASS xxxxx
  //
  // 2. Dummy weather data
  //    ダミー気象データ
  //
  //    shade,sun,humidity,pressure
  //
  // Only dummy weather data is used when sensors are disabled.
  // センサー無効時のみダミー気象データを使用する
  // ==========================================================

  if (Serial.available()) {
    // ========================================================
    // Read one line from USB serial
    // USBシリアルから1行読み込む
    // ========================================================
    String line = Serial.readStringUntil('\n');
    line.trim();
    // Remove spaces and CR/LF
    // 前後の空白や改行を削除
    // line.trim();

    // Ignore empty line
    // 空行は無視
    if (line.length() > 0) {

      // ======================================================
      // Configuration command
      // 設定コマンド
      // ======================================================
      if (line == "LOAD" || line.startsWith("SET ")) {

        handleConfigCommand(line);

      }
      else {

        // Dummy weather data
        // ダミー気象データ
        //
        // Format:
        // 書式:
        // shade,sun,humidity,pressure
        //
        // Example:
        // 例:
        // 28.5,35.2,78.0,1008.6

        int p1 = line.indexOf(',');
        int p2 = line.indexOf(',', p1 + 1);
        int p3 = line.indexOf(',', p2 + 1);

        if (p1 > 0 && p2 > p1 && p3 > p2) {

          shadeTemp = line.substring(0, p1).toFloat();
          sunTemp   = line.substring(p1 + 1, p2).toFloat();
          humidity  = line.substring(p2 + 1, p3).toFloat();
          pressure  = line.substring(p3 + 1).toFloat();

          Serial.println("DUMMY WEATHER UPDATED");
          Serial.print("shadeTemp = ");
          Serial.println(shadeTemp);
          Serial.print("sunTemp = ");
          Serial.println(sunTemp);
          Serial.print("humidity = ");
          Serial.println(humidity);
          Serial.print("pressure = ");
          Serial.println(pressure);
        }
        else {
          Serial.println("INVALID DUMMY DATA");
        }
      }
    }
  }

  // ==========================================================
  // RSSI monitor
  // RSSI監視
  // ----------------------------------------------------------
  // Print WiFi IP address and signal strength every 5 seconds
  // 5秒ごとにWiFiのIPアドレスと電波強度を表示
  // ==========================================================
  static unsigned long lastRSSI = 0;

  if (millis() - lastRSSI > 5000) {

    DBG_PRINT("IP=");
    DBG_PRINTLN(WiFi.localIP());

    DBG_PRINT("WiFi RSSI = ");
    DBG_PRINTLN(WiFi.RSSI());

    lastRSSI = millis();
  }
  
  // ==========================================================
  // Weather data upload timer
  // 気象データアップロードタイマー
  // ----------------------------------------------------------
  // Upload weather data once every minute
  // 1分ごとに気象データをアップロードする
  //
  // Upload timing:
  // アップロードタイミング:
  //
  // xx:00 ～ xx:02 秒の間に1回のみ実行
  // Execute only once between xx:00 and xx:02
  // ==========================================================

  static int lastUploadMinute = -1;

  struct tm timeinfo;


  // ==========================================================
  // Get current local time
  // 現在のローカル時刻取得
  // ==========================================================
  if (getLocalTime(&timeinfo)) {

    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;


    // ========================================================
    // Prevent duplicate uploads within the same minute
    // 同一分内の重複アップロード防止
    // --------------------------------------------------------
    // Execute only during first 2 seconds of each minute
    // 各分の最初の2秒間だけ実行
    // ========================================================
    if (second <= 2 && minute != lastUploadMinute) {

      char buf[32];


      // ======================================================
      // Create upload log message
      // アップロードログメッセージ作成
      // ======================================================
      sprintf(
        buf,
        "UPLOAD %02d:%02d:%02d",
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
      );

      DBG_PRINTLN(buf);


      // ======================================================
      // Save uploaded minute
      // アップロード済み分を記録
      // ======================================================
      lastUploadMinute = minute;


      // ======================================================
      // Upload weather data
      // 気象データアップロード
      // ======================================================
      uploadWeatherData();
    }
  }
  // ==========================================================
  // Heartbeat LED
  // ハートビートLED
  // ----------------------------------------------------------
  // Blink briefly every 3 seconds like a firefly
  // 3秒ごとにホタルのように短く点滅
  // ==========================================================

  static unsigned long lastLed = 0;

  if (millis() - lastLed >= 3000) {

    lastLed = millis();

    digitalWrite(PIN_LED, HIGH);

    delay(50);

    digitalWrite(PIN_LED, LOW);
  }
}