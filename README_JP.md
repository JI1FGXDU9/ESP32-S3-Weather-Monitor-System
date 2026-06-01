# README_JP.md

# ESP32 Weather Monitor System Ver0.0.1

* [English README](README.md)
* 日本語（このページ）

## 概要

ESP32-S3 を使用した気象観測システムです。

本システムは、

* 気温
* 湿度
* 気圧

をリアルタイムで測定し、Webサーバーへアップロードしてブラウザー上でグラフ表示します。

フィリピン・ミンダナオ島オザミス市の熱帯気候を観測する目的で製作しました。

特に、

* 日向の高温
* 夜間の高湿度
* 天候変化による気温変動

などをリアルタイムに記録できます。

---

# システム構成

## ESP32_Weather_Monitor_Ver0.0.1.ino

ESP32-S3側のメインプログラムです。

### 主な機能

* BME280 温湿度・気圧センサー読み取り
* DS18B20 温度センサー読み取り
* WiFi接続
* HTTP POSTによるデータ送信
* JSON形式データ生成
* 固定IP対応
* シリアル設定機能
* EEPROM設定保存

### 使用センサー

| センサー | 用途             |
| -------  | ---------------- |
| BME280   | 温度・湿度・気圧 |
| DS18B20  | 日向温度測定     |

### 測定内容

* 日陰温度
* 日向温度
* 湿度
* 気圧

---

# Weather_Monitor_Settings.py

ESP32の設定を変更するためのWindows用GUIツールです。

### 主な機能

* COMポート選択
* ESP32設定読み込み
* ESP32設定保存
* WiFi設定変更
* Upload URL変更
* 固定IP設定変更

### 必要ライブラリ

pyserial

インストール方法:

```bash
python -m pip install pyserial
```

### EXE化

```bash
pyinstaller --onefile --windowed --icon=weather_settings_icon.ico Weather_Monitor_Settings.py
```

---

# Web_Weather/upload.php

ESP32から送信されたデータを受信するPHPプログラムです。

### 主な機能

* ESP32からのHTTP POST受信
* JSONデータ保存
* CSV履歴保存
* Upload Key認証
* 最新データ更新

### 保存ファイル

| ファイル        | 内容       |
| -----------     | ---------- |
| data.json       | 最新データ |
| history.csv     | 履歴データ |

---

# Web_Weather/index.html

ブラウザーで気象データを表示するWebページです。

### 主な機能

* リアルタイムグラフ表示
* 日向・日陰温度表示
* 湿度表示
* 気圧表示
* Chart.js使用
* スマホ対応
* 日付切り替え表示
* 24時間グラフ表示

### 表示内容

| グラフ | 内容     |
| ---    | -------- |
| 赤線   | 日向温度 |
| 青線   | 日陰温度 |
| 緑線   | 湿度     |
| 黄線   | 気圧     |

---

# 動作環境

## ESP32側

* ESP32-S3
* Arduino IDE
* ESP32 Board Package

### 必要ライブラリ

* Adafruit BME280
* Adafruit Unified Sensor
* OneWire
* DallasTemperature

---

# サーバー側

* PHP 7以上
* Webサーバー (Apache等)

---

# 特徴

本システムは、単なる温度計ではなく、

* 日射
* 放射熱
* 湿度
* 熱帯夜

など、熱帯地域特有の環境変化をリアルタイムに観測できます。

特にフィリピンでは、

* 夜間湿度80%以上
* 日向温度50℃以上

になることがあり、非常に興味深いデータを取得できます。

---

# ライブデータ

https://ji1fgx.com/weather/index.html

---

# 詳細解説

https://ji1fgx.com/260531.html

---

# ライセンス

個人利用・改造自由。

ただし自己責任で使用してください。

---

上野浩一
JI1FGX/DU9
du9@ji1fgx.com

