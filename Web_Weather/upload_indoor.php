<?php
// upload_indoor.php
// ============================================================
// Weather data upload receiver
// 気象データアップロード受信プログラム
// ------------------------------------------------------------
// File name:
// ファイル名:
//
// upload.php
//
// Receives JSON data from ESP32,
// saves the latest data to data.json,
// and appends 10-minute interval history to history.csv.
//
// ESP32からJSONデータを受信し、
// 最新データを data.json に保存し、
// 10分間隔の履歴を history.csv に追記する。
// ============================================================


// ============================================================
// Time zone setting
// タイムゾーン設定
// ------------------------------------------------------------
// Philippines time
// フィリピン時間
// ============================================================
date_default_timezone_set("Asia/Manila");


// ============================================================
// Upload authentication key
// アップロード認証キー
// ------------------------------------------------------------
// Must match ESP32-side uploadKey
// ESP32側の uploadKey と一致させる
// ============================================================
$secret_key = "test";


// ============================================================
// Read raw POST body
// POST生データ読み込み
// ------------------------------------------------------------
// ESP32 sends JSON in HTTP request body
// ESP32はHTTPリクエスト本文にJSONを送信する
// ============================================================
$raw = file_get_contents("php://input");


// ============================================================
// Decode JSON
// JSONデコード
// ============================================================
$data = json_decode($raw, true);


// ============================================================
// JSON error check
// JSONエラーチェック
// ============================================================
if ($data === null) {

    http_response_code(400);

    echo "JSON ERROR";

    exit;
}


// ============================================================
// Upload key check
// アップロードキー確認
// ============================================================
if (!isset($data["key"]) || $data["key"] !== $secret_key) {

    http_response_code(403);

    echo "KEY ERROR";

    exit;
}

function fmtSensor($value) {

    if ($value === null) {
        return "";
    }

    if (!is_numeric($value)) {
        return "";
    }

    return number_format((float)$value, 1, ".", "");
}

// ============================================================
// Create output data
// 出力データ作成
// ------------------------------------------------------------
// number_format(..., 1, ".", "")
//   Force one decimal place
//   小数点1桁に統一
// ============================================================
$out = array(

    // Server receive time
    // サーバー受信時刻
    "time"        => date("Y-m-d H:i:s"),

    "shade_temp"  => fmtSensor($data["shade_temp"]),

    "sun_temp"    => fmtSensor($data["sun_temp"]),

    "humidity"    => fmtSensor($data["humidity"]),

    "pressure"    => fmtSensor($data["pressure"])
);


// ============================================================
// Save latest JSON
// 最新JSON保存
// ------------------------------------------------------------
// data.json is used by the web page to display current values
// data.json はWebページの現在値表示に使用する
// ============================================================
$result = file_put_contents(

    __DIR__ . "/indoor.json",

    json_encode($out, JSON_PRETTY_PRINT)
);


// ============================================================
// Check latest JSON write result
// 最新JSON書き込み確認
// ============================================================
if ($result === false) {

    http_response_code(500);

    echo "WRITE ERROR";

    exit;
}


// ============================================================
// Append history CSV
// 履歴CSV追記
// ============================================================
$csvfile = __DIR__ . "/history_indoor.csv";


// ============================================================
// Create CSV line
// CSV行作成
// ------------------------------------------------------------
// Format:
// 形式:
//
// date,time,shade_temp,sun_temp,humidity,pressure
// ============================================================
$csvDate = date("Y/n/j");  // 2026/5/30
$csvTime = date("H:i");    // 21:20

$line =
    $csvDate . "," .
    $csvTime . "," .
    $out["shade_temp"] . "," .
    $out["sun_temp"] . "," .
    $out["humidity"] . "," .
    $out["pressure"] . "\n";


// ============================================================
// Create CSV header if file does not exist
// CSVファイルが無ければヘッダーを作成
// ============================================================
if (!file_exists($csvfile)) {

    $header =
        "date,time,shade_temp,sun_temp,humidity,pressure\n";

    file_put_contents($csvfile, $header);
}


// ============================================================
// 10-minute slot control
// 10分スロット制御
// ------------------------------------------------------------
// Prevents writing multiple history records
// within the same 10-minute period.
//
// 同じ10分間に複数の履歴レコードを書かないようにする。
// ============================================================
$slotfile = __DIR__ . "/last_history_slot_indoor.txt";


// ============================================================
// Create current 10-minute slot ID
// 現在の10分スロットID作成
// ------------------------------------------------------------
// Example:
// 例:
//
// 2026-05-26 13:0  -> 13:00-13:09
// 2026-05-26 13:1  -> 13:10-13:19
// 2026-05-26 13:2  -> 13:20-13:29
// ============================================================
$slot = date("Y-m-d H:") . floor(intval(date("i")) / 10);


// ============================================================
// Read last written slot
// 前回書き込み済みスロット読み込み
// ============================================================
$lastSlot = "";

if (file_exists($slotfile)) {

    $lastSlot = trim(file_get_contents($slotfile));
}


// ============================================================
// Append history only once per 10-minute slot
// 10分スロットごとに1回だけ履歴追記
// ============================================================
if ($slot !== $lastSlot) {

    // Append CSV line
    // CSV行を追記
    file_put_contents(
        $csvfile,
        $line,
        FILE_APPEND
    );

    // Save current slot as last written slot
    // 現在スロットを前回書き込み済みとして保存
    file_put_contents($slotfile, $slot);
}


// ============================================================
// Response to ESP32
// ESP32への応答
// ============================================================
echo "OK";

?>