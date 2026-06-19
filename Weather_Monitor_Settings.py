# ==========================================================
# Weather_Monitor_Settings.py
# ESP32 Weather Monitor Settings Tool
# ESP32 Weather Monitor 設定ツール
# ----------------------------------------------------------
# Build EXE:
# EXE作成:
# pyinstaller --onefile --windowed --icon=weather_settings_icon.ico Weather_Monitor_Settings.py
#
# pyserial library is required
# pyserial ライブラリが必要です
# Install:
# インストール:
# python -m pip install pyserial
# ==========================================================
from tkinter import ttk
from tkinter import messagebox
import tkinter as tk

import serial
import serial.tools.list_ports
import time
import configparser
import os

# ==========================================================
# Serial communication settings
# シリアル通信設定
# ==========================================================
BAUDRATE = 115200

# ==========================================================
# INI file name
# INIファイル名
# ==========================================================
INI_FILE = "settings.ini"

# ==========================================================
# Configuration items
# 設定項目
# ==========================================================
FIELDS = [
    "SSID",
    "PASS",
    "UPLOAD_URL",
    "UPLOAD_KEY",
    "LOCAL_IP",
    "GATEWAY",
    "SUBNET",
]

# ==========================================================
# Sensor mode list
# センサーモード一覧
# ==========================================================
SENSOR_MODES = {
    "0 : No Sensor": "0",
    "1 : BME280": "1",
    "2 : DS18B20": "2",
    "3 : BME280 + DS18B20": "3",
}

config = configparser.ConfigParser()

entries = {}

sensor_var = None
# ==========================================================
# Load INI file
# INIファイル読み込み
# ==========================================================
def load_ini():
    if os.path.exists(INI_FILE):
        config.read(INI_FILE, encoding="utf-8")

# ==========================================================
# Save INI file
# INIファイル保存
# ----------------------------------------------------------
# Saves the last used COM port
# 最後に使用したCOMポートを保存
# ==========================================================
def save_ini():
    config["SETTING"] = {
        "SERIAL_PORT": port_var.get()
    }

    with open(INI_FILE, "w", encoding="utf-8") as f:
        config.write(f)

# ==========================================================
# Get available COM ports
# 使用可能COMポート取得
# ==========================================================
def get_ports():
    return [p.device for p in serial.tools.list_ports.comports()]

# ==========================================================
# Send one line to ESP32
# ESP32へ1行送信
# ==========================================================
def send_line(ser, line):
    ser.write((line + "\n").encode("utf-8"))
    time.sleep(0.1)

# ==========================================================
# Load settings from ESP32
# ESP32から設定読み込み
# ==========================================================
def load_settings():
    port = port_var.get()

    # ------------------------------------------------------
    # COM port not selected
    # COMポート未選択
    # ------------------------------------------------------
    if not port:
        messagebox.showerror("ERROR", "COMポートが選択されていません")
        return

    try:
        with serial.Serial(port, BAUDRATE, timeout=2) as ser:

            # --------------------------------------------------
            # Wait for ESP32 reset after opening serial port
            # シリアルポートオープン後ESP32リセット待ち
            # --------------------------------------------------
            time.sleep(2)

            ser.reset_input_buffer()

            # --------------------------------------------------
            # Send LOAD command
            # LOADコマンド送信
            # --------------------------------------------------
            send_line(ser, "LOAD")
            start = time.time()
            
            while True:
                if time.time() - start > 5:
                    messagebox.showerror("ERROR", "LOAD timeout")
                    return
            
                line = ser.readline().decode(errors="ignore").strip()
            
                if line == "":
                    continue
            
                print("LOAD:", line)
            
                if line == "END":
                    break
            
                if "=" in line:
                    key, val = line.split("=", 1)
            
                    if key in entries:
                        entries[key].delete(0, tk.END)
                        entries[key].insert(0, val)
            
                    elif key == "SENSOR":
                        for label, value in SENSOR_MODES.items():
                            if value == val:
                                sensor_var.set(label)
                                break
        save_ini()

        messagebox.showinfo(
            "LOAD",
            "設定を読み込みました"
        )

    except Exception as e:
        messagebox.showerror("ERROR", str(e))

# ==========================================================
# Wait for OK response
# OK応答待ち
# ==========================================================
def wait_ok(ser, timeout=5):

    start = time.time()

    while time.time() - start < timeout:

        line = ser.readline().decode(errors="ignore").strip()

        if line == "":
            continue

        print("RX:", line)

        # ------------------------------------------------------
        # OK received
        # OK受信
        # ------------------------------------------------------
        if line == "OK":
            return True

    return False

# ==========================================================
# Save settings to ESP32
# ESP32へ設定保存
# ==========================================================
def save_settings():

    port = port_var.get()

    # ------------------------------------------------------
    # COM port not selected
    # COMポート未選択
    # ------------------------------------------------------
    if not port:
        messagebox.showerror(
            "ERROR",
            "COMポートが選択されていません"
        )
        return

    try:
        with serial.Serial(port, BAUDRATE, timeout=1) as ser:

            # --------------------------------------------------
            # Wait for ESP32 reset
            # ESP32リセット待ち
            # --------------------------------------------------
            time.sleep(2)

            ser.reset_input_buffer()

            # --------------------------------------------------
            # Send text entry settings
            # 文字入力設定を送信
            # --------------------------------------------------
            for key in FIELDS:

                val = entries[key].get()
                
                if val == "":
                    val = "__EMPTY__"
                
                send_line(
                    ser,
                    f"SET {key} {val}"
                )

                # --------------------------------------------------
                # Wait for OK response
                # OK応答待ち
                # --------------------------------------------------
                if not wait_ok(ser):

                    messagebox.showerror(
                        "ERROR",
                        f"{key} 保存失敗"
                    )

                    return

            # --------------------------------------------------
            # Send sensor mode setting
            # センサーモード設定を送信
            # --------------------------------------------------
            sensor_value = SENSOR_MODES.get(
                sensor_var.get(),
                "3"
            )

            send_line(
                ser,
                f"SET SENSOR {sensor_value}"
            )

            # --------------------------------------------------
            # Wait for OK response
            # OK応答待ち
            # --------------------------------------------------
            if not wait_ok(ser):

                messagebox.showerror(
                    "ERROR",
                    "SENSOR 保存失敗"
                )

                return

        save_ini()

        messagebox.showinfo(
            "SAVE",
            "設定を保存しました"
        )

    except Exception as e:
        messagebox.showerror("ERROR", str(e))

# ==========================================================
# Refresh COM port list
# COMポート一覧更新
# ==========================================================
def refresh_ports():

    ports = get_ports()

    menu = port_menu["menu"]
    menu.delete(0, "end")

    for p in ports:

        menu.add_command(
            label=p,
            command=lambda value=p: port_var.set(value)
        )

    # ------------------------------------------------------
    # Restore last used COM port
    # 前回使用COMポート復元
    # ------------------------------------------------------
    saved_port = config.get(
        "SETTING",
        "SERIAL_PORT",
        fallback=""
    )

    if saved_port in ports:
        port_var.set(saved_port)

    elif ports:
        port_var.set(ports[0])

    else:
        port_var.set("")

# ==========================================================
# Load INI file at startup
# 起動時INI読み込み
# ==========================================================
load_ini()

# ==========================================================
# Create main window
# メインウィンドウ作成
# ==========================================================
root = tk.Tk()

root.title("ESP32 Weather Monitor Settings")

# ==========================================================
# COM Port label
# COMポートラベル
# ==========================================================
tk.Label(
    root,
    text="COM Port"
).grid(
    row=0,
    column=0,
    padx=5,
    pady=5,
    sticky="e"
)

port_var = tk.StringVar()

ports = get_ports()

saved_port = config.get(
    "SETTING",
    "SERIAL_PORT",
    fallback=""
)

# ==========================================================
# Restore COM port
# COMポート復元
# ==========================================================
if saved_port in ports:
    port_var.set(saved_port)

elif ports:
    port_var.set(ports[0])

else:
    port_var.set("")

# ==========================================================
# COM port selection menu
# COMポート選択メニュー
# ==========================================================
port_menu = tk.OptionMenu(
    root,
    port_var,
    *ports
)

port_menu.grid(
    row=0,
    column=1,
    padx=5,
    pady=5,
    sticky="w"
)

# ==========================================================
# Refresh button
# 更新ボタン
# ==========================================================
tk.Button(
    root,
    text="Refresh",
    width=10,
    command=refresh_ports
).grid(
    row=0,
    column=2,
    padx=5,
    pady=5
)

# ==========================================================
# Create setting fields
# 設定項目作成
# ==========================================================
for i, key in enumerate(FIELDS, start=1):

    tk.Label(
        root,
        text=key
    ).grid(
        row=i,
        column=0,
        padx=5,
        pady=5,
        sticky="e"
    )

    ent = tk.Entry(
        root,
        width=45
    )

    ent.grid(
        row=i,
        column=1,
        columnspan=2,
        padx=5,
        pady=5
    )

    entries[key] = ent

# ==========================================================
# Sensor mode combobox
# センサーモード選択コンボボックス
# ==========================================================
sensor_row = len(FIELDS) + 1

tk.Label(
    root,
    text="SENSOR"
).grid(
    row=sensor_row,
    column=0,
    padx=5,
    pady=5,
    sticky="e"
)

sensor_var = tk.StringVar()

sensor_var.set(
    "3 : BME280 + DS18B20"
)

sensor_combo = ttk.Combobox(
    root,
    textvariable=sensor_var,
    values=list(SENSOR_MODES.keys()),
    width=42,
    state="readonly"
)

sensor_combo.grid(
    row=sensor_row,
    column=1,
    columnspan=2,
    padx=5,
    pady=5
)

# ==========================================================
# LOAD button
# LOADボタン
# ==========================================================
tk.Button(
    root,
    text="LOAD",
    width=15,
    command=load_settings
).grid(
    row=len(FIELDS) + 2,
    column=0,
    padx=5,
    pady=10
)

# ==========================================================
# SAVE button
# SAVEボタン
# ==========================================================
tk.Button(
    root,
    text="SAVE",
    width=15,
    command=save_settings
).grid(
    row=len(FIELDS) + 2,
    column=1,
    padx=5,
    pady=10
)

# ==========================================================
# Auto LOAD after startup
# 起動後自動LOAD
# ----------------------------------------------------------
# Loads settings from the last used COM port saved in settings.ini
# settings.ini に保存された前回のCOMポートから設定を読み込む
# ==========================================================
###root.after(500, load_settings)

# ==========================================================
# Start GUI main loop
# GUIメインループ開始
# ==========================================================
root.mainloop()
