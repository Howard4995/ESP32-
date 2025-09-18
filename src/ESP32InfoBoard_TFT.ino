/*
 * ESP32 桌面信息牌（iOS Widgets 風格）
 * 
 * 硬體需求：
 * - ESP32 開發板
 * - 2.8" ILI9341 TFT 320×240 SPI 顯示器
 * 
 * 接線（可修改）：
 * TFT_SCK  = 18
 * TFT_MISO = 19  
 * TFT_MOSI = 23
 * TFT_CS   = 5
 * TFT_DC   = 2
 * TFT_RST  = 4
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <time.h>
#include <BluetoothSerial.h>
#include "icons_tft.h"

// TFT 接腳定義
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
#define TFT_MOSI 23
#define TFT_SCK  18
#define TFT_MISO 19

// 顏色定義
#define COLOR_BG      0x0861    // 深藍灰
#define COLOR_WIDGET  0x2124    // 深灰
#define COLOR_TEXT    0xFFFF    // 白色
#define COLOR_MUTED   0x8410    // 淺灰
#define COLOR_ACCENT  0x051D    // 藍色
#define COLOR_ORANGE  0xFD20    // 橙色

// WiFi AP 設定
const char* AP_SSID = "ESP32-InfoBoard-Setup";
const char* AP_PASSWORD = "12345678";

// 全域變數
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
WebServer server(80);
Preferences prefs;
BluetoothSerial SerialBT;

String wifi_ssid = "";
String wifi_password = "";
String qweather_key = "";
String city_name = "";
String city_id = "";

// 天氣資料結構
struct WeatherData {
  String temp = "--";
  String text = "載入中";
  String humidity = "--";
  String windDir = "--";
  String windScale = "--";
  IconType icon = ICON_UNKNOWN;
  unsigned long lastUpdate = 0;
};

struct ForecastData {
  String date = "--";
  String tempMax = "--";
  String tempMin = "--";
  IconType icon = ICON_UNKNOWN;
};

struct WarningData {
  String text = "";
  String level = "";
  bool active = false;
  unsigned long lastUpdate = 0;
};

// 系統監控資料結構
struct SystemMonitorData {
  float cpuUsage = 0.0;
  float gpuUsage = 0.0;
  float ramUsage = 0.0;
  float ramTotal = 0.0;
  float ramUsed = 0.0;
  String cpuTemp = "--";
  String gpuTemp = "--";
  bool connected = false;
  unsigned long lastUpdate = 0;
};

WeatherData currentWeather;
ForecastData forecast[3];
WarningData warning;
SystemMonitorData systemData;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 桌面信息牌啟動中...");
  
  // 初始化 TFT
  tft.begin();
  tft.setRotation(1);  // 橫向顯示 320x240
  tft.fillScreen(COLOR_BG);
  
  // 顯示啟動畫面
  showStartupScreen();
  
  // 初始化偏好設定
  prefs.begin("weather", false);
  loadSettings();
  
  // 初始化藍牙
  SerialBT.begin("ESP32-InfoBoard"); // 藍牙裝置名稱
  Serial.println("藍牙已啟動，等待連線...");
  
  // 設定時區
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  // WiFi 連線
  if (wifi_ssid.length() > 0) {
    connectWiFi();
  } else {
    startAPMode();
  }
  
  // 設定 Web 伺服器路由
  setupWebServer();
  
  // 初始化顯示
  drawInterface();
  
  Serial.println("系統就緒");
}

void loop() {
  server.handleClient();
  
  static unsigned long lastWeatherUpdate = 0;
  static unsigned long lastTimeUpdate = 0;
  static unsigned long lastWarningUpdate = 0;
  static unsigned long lastSystemUpdate = 0;
  
  unsigned long now = millis();
  
  // 處理藍牙接收的系統監控資料
  handleBluetoothData();
  
  // 每 10 分鐘更新天氣
  if (now - lastWeatherUpdate > 600000 || lastWeatherUpdate == 0) {
    if (WiFi.status() == WL_CONNECTED && qweather_key.length() > 0 && city_id.length() > 0) {
      updateWeather();
      updateForecast();
      lastWeatherUpdate = now;
    }
  }
  
  // 每 30 分鐘更新預警
  if (now - lastWarningUpdate > 1800000 || lastWarningUpdate == 0) {
    if (WiFi.status() == WL_CONNECTED && qweather_key.length() > 0 && city_id.length() > 0) {
      updateWarning();
      lastWarningUpdate = now;
    }
  }
  
  // 每秒更新時間顯示
  if (now - lastTimeUpdate > 1000) {
    updateTimeDisplay();
    lastTimeUpdate = now;
  }
  
  // 每 2 秒更新系統監控顯示
  if (now - lastSystemUpdate > 2000) {
    updateSystemMonitorDisplay();
    lastSystemUpdate = now;
  }
  
  delay(100);
}

void showStartupScreen() {
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  
  int16_t x = (320 - 12 * 2 * 6) / 2;  // 居中
  int16_t y = 100;
  
  tft.setCursor(x, y);
  tft.println("ESP32 信息牌");
  
  tft.setTextSize(1);
  tft.setCursor(x + 20, y + 40);
  tft.println("iOS Widgets 風格");
  
  delay(2000);
}

void loadSettings() {
  wifi_ssid = prefs.getString("wifi_ssid", "");
  wifi_password = prefs.getString("wifi_pwd", "");
  qweather_key = prefs.getString("qw_key", "");
  city_name = prefs.getString("city_name", "");
  city_id = prefs.getString("city_id", "");
  
  Serial.println("設定載入完成");
}

void saveSettings() {
  prefs.putString("wifi_ssid", wifi_ssid);
  prefs.putString("wifi_pwd", wifi_password);
  prefs.putString("qw_key", qweather_key);
  prefs.putString("city_name", city_name);
  prefs.putString("city_id", city_id);
  
  Serial.println("設定已儲存");
}

void connectWiFi() {
  Serial.print("連線到 WiFi: ");
  Serial.println(wifi_ssid);
  
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi 已連線，IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi 連線失敗，啟動 AP 模式");
    startAPMode();
  }
}

void startAPMode() {
  Serial.println("啟動 AP 模式");
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(IP);
  
  // 顯示設定資訊
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  
  tft.setCursor(10, 50);
  tft.println("WiFi 設定模式");
  tft.setCursor(10, 70);
  tft.print("SSID: ");
  tft.println(AP_SSID);
  tft.setCursor(10, 90);
  tft.print("密碼: ");
  tft.println(AP_PASSWORD);
  tft.setCursor(10, 110);
  tft.print("設定頁: http://");
  tft.println(IP);
}

void setupWebServer() {
  // 設定頁面
  server.on("/", []() {
    String html = getConfigHTML();
    server.send(200, "text/html", html);
  });
  
  // 儲存設定
  server.on("/api/save", HTTP_POST, []() {
    if (server.hasArg("wifi_ssid")) wifi_ssid = server.arg("wifi_ssid");
    if (server.hasArg("wifi_password")) wifi_password = server.arg("wifi_password");
    if (server.hasArg("qweather_key")) qweather_key = server.arg("qweather_key");
    if (server.hasArg("city_name")) {
      city_name = server.arg("city_name");
      // 查詢城市 ID
      city_id = getCityId(city_name);
    }
    
    saveSettings();
    
    server.send(200, "application/json", "{\"status\":\"success\"}");
    
    delay(1000);
    ESP.restart();
  });
  
  // API 端點
  server.on("/api/weather", []() {
    JsonDocument doc;
    doc["temp"] = currentWeather.temp;
    doc["text"] = currentWeather.text;
    doc["humidity"] = currentWeather.humidity;
    doc["wind"] = currentWeather.windDir + " " + currentWeather.windScale + "級";
    
    JsonArray forecastArray = doc["forecast"].to<JsonArray>();
    for (int i = 0; i < 3; i++) {
      JsonDocument item;
      item["date"] = forecast[i].date;
      item["tempMax"] = forecast[i].tempMax;
      item["tempMin"] = forecast[i].tempMin;
      forecastArray.add(item);
    }
    
    if (warning.active) {
      doc["warning"]["text"] = warning.text;
      doc["warning"]["level"] = warning.level;
    }
    
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
  });
  
  server.begin();
  Serial.println("Web 伺服器已啟動");
}

String getConfigHTML() {
  return R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>ESP32 信息牌設定</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="password"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background: #007AFF; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; }
        button:hover { background: #0056CC; }
        .info { background: #e7f3ff; padding: 10px; border-radius: 4px; margin-bottom: 20px; font-size: 14px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 桌面信息牌</h1>
        <div class="info">
            <strong>設定說明：</strong><br>
            • WiFi：設定網路連線<br>
            • API Key：到 <a href="https://dev.qweather.com" target="_blank">和風天氣</a> 免費註冊<br>
            • 城市：可用中文、英文或拼音
        </div>
        
        <form onsubmit="saveConfig(event)">
            <div class="form-group">
                <label>WiFi SSID：</label>
                <input type="text" name="wifi_ssid" required>
            </div>
            <div class="form-group">
                <label>WiFi 密碼：</label>
                <input type="password" name="wifi_password">
            </div>
            <div class="form-group">
                <label>和風天氣 API Key：</label>
                <input type="text" name="qweather_key" required>
            </div>
            <div class="form-group">
                <label>城市名稱：</label>
                <input type="text" name="city_name" placeholder="例：台北、Beijing、shanghai" required>
            </div>
            <button type="submit">保存設定並重啟</button>
        </form>
    </div>
    
    <script>
        function saveConfig(event) {
            event.preventDefault();
            const form = event.target;
            const formData = new FormData(form);
            
            fetch('/api/save', {
                method: 'POST',
                body: new URLSearchParams(formData)
            })
            .then(response => response.json())
            .then(data => {
                alert('設定已保存，裝置將重啟...');
            })
            .catch(error => {
                alert('保存失敗：' + error);
            });
        }
    </script>
</body>
</html>
)";
}

String getCityId(String cityName) {
  if (qweather_key.length() == 0) return "";
  
  WiFiClientSecure client;
  client.setInsecure();
  
  String url = "https://geoapi.qweather.com/v2/city/lookup?location=" + 
               cityName + "&key=" + qweather_key;
  
  if (client.connect("geoapi.qweather.com", 443)) {
    client.print("GET " + url.substring(27) + " HTTP/1.1\r\n");
    client.print("Host: geoapi.qweather.com\r\n");
    client.print("Connection: close\r\n\r\n");
    
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();
    
    int jsonStart = response.indexOf("\r\n\r\n");
    if (jsonStart != -1) {
      String jsonResponse = response.substring(jsonStart + 4);
      
      JsonDocument doc;
      if (deserializeJson(doc, jsonResponse) == DeserializationError::Ok) {
        if (doc["code"] == "200" && doc["location"].size() > 0) {
          return doc["location"][0]["id"].as<String>();
        }
      }
    }
  }
  
  return "";
}

void updateWeather() {
  if (city_id.length() == 0) return;
  
  WiFiClientSecure client;
  client.setInsecure();
  
  String url = "/v7/weather/now?location=" + city_id + "&key=" + qweather_key;
  
  if (client.connect("devapi.qweather.com", 443)) {
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: devapi.qweather.com\r\n");
    client.print("Connection: close\r\n\r\n");
    
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();
    
    int jsonStart = response.indexOf("\r\n\r\n");
    if (jsonStart != -1) {
      String jsonResponse = response.substring(jsonStart + 4);
      
      JsonDocument doc;
      if (deserializeJson(doc, jsonResponse) == DeserializationError::Ok) {
        if (doc["code"] == "200") {
          auto now = doc["now"];
          currentWeather.temp = now["temp"].as<String>();
          currentWeather.text = now["text"].as<String>();
          currentWeather.humidity = now["humidity"].as<String>();
          currentWeather.windDir = now["windDir"].as<String>();
          currentWeather.windScale = now["windScale"].as<String>();
          currentWeather.icon = getIconType(now["icon"].as<String>());
          currentWeather.lastUpdate = millis();
          
          Serial.println("天氣更新完成");
          drawWeatherCard();
        }
      }
    }
  }
}

void updateForecast() {
  if (city_id.length() == 0) return;
  
  WiFiClientSecure client;
  client.setInsecure();
  
  String url = "/v7/weather/3d?location=" + city_id + "&key=" + qweather_key;
  
  if (client.connect("devapi.qweather.com", 443)) {
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: devapi.qweather.com\r\n");
    client.print("Connection: close\r\n\r\n");
    
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();
    
    int jsonStart = response.indexOf("\r\n\r\n");
    if (jsonStart != -1) {
      String jsonResponse = response.substring(jsonStart + 4);
      
      JsonDocument doc;
      if (deserializeJson(doc, jsonResponse) == DeserializationError::Ok) {
        if (doc["code"] == "200") {
          auto daily = doc["daily"];
          for (int i = 0; i < 3 && i < daily.size(); i++) {
            String date = daily[i]["fxDate"].as<String>();
            forecast[i].date = date.substring(5);  // MM-DD
            forecast[i].tempMax = daily[i]["tempMax"].as<String>();
            forecast[i].tempMin = daily[i]["tempMin"].as<String>();
            forecast[i].icon = getIconType(daily[i]["iconDay"].as<String>());
          }
          
          Serial.println("預報更新完成");
          drawForecastCards();
        }
      }
    }
  }
}

void updateWarning() {
  if (city_id.length() == 0) return;
  
  WiFiClientSecure client;
  client.setInsecure();
  
  String url = "/v7/warning/now?location=" + city_id + "&key=" + qweather_key;
  
  if (client.connect("devapi.qweather.com", 443)) {
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: devapi.qweather.com\r\n");
    client.print("Connection: close\r\n\r\n");
    
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();
    
    int jsonStart = response.indexOf("\r\n\r\n");
    if (jsonStart != -1) {
      String jsonResponse = response.substring(jsonStart + 4);
      
      JsonDocument doc;
      if (deserializeJson(doc, jsonResponse) == DeserializationError::Ok) {
        if (doc["code"] == "200") {
          auto warningArray = doc["warning"];
          if (warningArray.size() > 0) {
            warning.text = warningArray[0]["text"].as<String>();
            warning.level = warningArray[0]["level"].as<String>();
            warning.active = true;
          } else {
            warning.active = false;
          }
          warning.lastUpdate = millis();
          
          Serial.println("預警更新完成");
          drawWarningBar();
        }
      }
    }
  }
}

IconType getIconType(String iconCode) {
  int code = iconCode.toInt();
  
  if (code >= 100 && code <= 103) return ICON_SUNNY;
  if (code >= 104 && code <= 213) return ICON_CLOUDY;
  if (code >= 300 && code <= 318) return ICON_RAIN;
  if (code >= 400 && code <= 410) return ICON_SNOW;
  if (code >= 500 && code <= 515) return ICON_FOG;
  if (code >= 600 && code <= 610) return ICON_THUNDER;
  
  return ICON_UNKNOWN;
}

void drawInterface() {
  tft.fillScreen(COLOR_BG);
  
  // 繪製狀態列
  drawStatusBar();
  
  // 繪製時鐘卡片
  drawClockCard();
  
  // 繪製天氣卡片
  drawWeatherCard();
  
  // 繪製系統監控卡片（新增）
  drawSystemMonitorCards();
  
  // 繪製預警列
  drawWarningBar();
}

void drawStatusBar() {
  // 狀態列背景
  tft.fillRect(0, 0, 320, 24, COLOR_WIDGET);
  
  // 城市名稱
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.setCursor(8, 8);
  if (city_name.length() > 0) {
    tft.print(city_name);
  } else {
    tft.print("未設定城市");
  }
  
  // IP 地址
  String ipText = "IP ";
  if (WiFi.status() == WL_CONNECTED) {
    ipText += WiFi.localIP().toString();
  } else {
    ipText += WiFi.softAPIP().toString();
  }
  
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(ipText, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(320 - w - 8, 8);
  tft.print(ipText);
}

void drawClockCard() {
  // 卡片背景
  tft.fillRoundRect(8, 32, 182, 102, 12, COLOR_WIDGET);
  
  // 標題
  tft.setTextColor(COLOR_MUTED);
  tft.setTextSize(1);
  tft.setCursor(16, 42);
  tft.print("時鐘");
  
  updateTimeDisplay();
}

void updateTimeDisplay() {
  // 清除時間區域
  tft.fillRect(16, 55, 166, 70, COLOR_WIDGET);
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    // 時間
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(4);
    tft.setCursor(16, 60);
    tft.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    
    // 日期
    tft.setTextColor(COLOR_MUTED);
    tft.setTextSize(1);
    tft.setCursor(16, 110);
    
    const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    tft.printf("%02d-%02d %s", 
               timeinfo.tm_mon + 1, 
               timeinfo.tm_mday, 
               weekdays[timeinfo.tm_wday]);
  } else {
    tft.setTextColor(COLOR_MUTED);
    tft.setTextSize(2);
    tft.setCursor(16, 70);
    tft.print("--:--");
  }
}

void drawWeatherCard() {
  // 卡片背景
  tft.fillRoundRect(198, 32, 114, 102, 12, COLOR_WIDGET);
  
  // 標題
  tft.setTextColor(COLOR_MUTED);
  tft.setTextSize(1);
  tft.setCursor(206, 42);
  tft.print("現在天氣");
  
  // 圖示和溫度
  drawWeatherIconLarge(tft, currentWeather.icon, 206, 55, COLOR_ACCENT);
  
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.setCursor(258, 65);
  tft.print(currentWeather.temp);
  tft.print("°");
  
  // 天氣描述
  tft.setTextColor(COLOR_MUTED);
  tft.setTextSize(1);
  tft.setCursor(206, 105);
  tft.print(currentWeather.text);
  
  // 濕度和風力
  tft.setCursor(206, 118);
  tft.print("濕度 ");
  tft.print(currentWeather.humidity);
  tft.print("%");
  
  if (currentWeather.windDir.length() > 0) {
    tft.print(" · ");
    tft.print(currentWeather.windDir);
    tft.print(" ");
    tft.print(currentWeather.windScale);
    tft.print("級");
  }
}

void drawForecastCards() {
  for (int i = 0; i < 3; i++) {
    int x = 8 + i * 102;
    int y = 142;
    
    // 卡片背景
    tft.fillRoundRect(x, y, 96, 60, 8, COLOR_WIDGET);
    
    // 日期
    tft.setTextColor(COLOR_MUTED);
    tft.setTextSize(1);
    tft.setCursor(x + 8, y + 8);
    tft.print(forecast[i].date);
    
    // 圖示
    drawWeatherIconSmall(tft, forecast[i].icon, x + 8, y + 20, COLOR_ACCENT);
    
    // 溫度
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.setCursor(x + 40, y + 25);
    tft.print(forecast[i].tempMin);
    tft.print("~");
    tft.print(forecast[i].tempMax);
    tft.print("°");
  }
}

void drawWarningBar() {
  if (warning.active && warning.text.length() > 0) {
    // 預警背景
    uint16_t warningColor = COLOR_ORANGE;
    if (warning.level == "紅色" || warning.level == "Red") {
      warningColor = ILI9341_RED;
    } else if (warning.level == "黃色" || warning.level == "Yellow") {
      warningColor = ILI9341_YELLOW;
    }
    
    tft.fillRoundRect(8, 235, 304, 22, 11, warningColor);
    
    // 預警文字
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(1);
    tft.setCursor(16, 243);
    
    // 截斷過長文字
    String displayText = warning.text;
    if (displayText.length() > 35) {
      displayText = displayText.substring(0, 32) + "...";
    }
    tft.print(displayText);
  } else {
    // 清除預警區域
    tft.fillRect(8, 235, 304, 22, COLOR_BG);
  }
}

// 處理藍牙接收的系統監控資料
void handleBluetoothData() {
  if (SerialBT.available()) {
    String receivedData = SerialBT.readStringUntil('\n');
    receivedData.trim();
    
    if (receivedData.length() > 0) {
      parseSystemData(receivedData);
    }
  }
  
  // 檢查連線狀態（超過 10 秒沒收到資料視為斷線）
  if (millis() - systemData.lastUpdate > 10000) {
    systemData.connected = false;
  }
}

// 解析系統監控資料
void parseSystemData(String jsonData) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    Serial.println("JSON 解析錯誤");
    return;
  }
  
  if (doc["type"] == "system") {
    systemData.cpuUsage = doc["cpu_usage"];
    systemData.gpuUsage = doc["gpu_usage"];
    systemData.ramUsage = doc["ram_usage"];
    systemData.ramTotal = doc["ram_total"];
    systemData.ramUsed = doc["ram_used"];
    systemData.cpuTemp = doc["cpu_temp"].as<String>();
    systemData.gpuTemp = doc["gpu_temp"].as<String>();
    systemData.connected = true;
    systemData.lastUpdate = millis();
    
    Serial.println("系統監控資料已更新");
  }
}

// 更新系統監控顯示
void updateSystemMonitorDisplay() {
  drawSystemMonitorCards();
}

// 繪製系統監控卡片
void drawSystemMonitorCards() {
  // CPU 監控卡片 (左)
  drawSystemCard(8, 142, "CPU", systemData.cpuUsage, systemData.cpuTemp, COLOR_ACCENT);
  
  // GPU 監控卡片 (中)
  drawSystemCard(112, 142, "GPU", systemData.gpuUsage, systemData.gpuTemp, 0x07E0); // 綠色
  
  // RAM 監控卡片 (右)
  String ramInfo = String(systemData.ramUsed, 1) + "/" + String(systemData.ramTotal, 1) + "GB";
  drawSystemCard(216, 142, "RAM", systemData.ramUsage, ramInfo, 0xF81F); // 紫色
}

// 繪製單個系統監控卡片
void drawSystemCard(int x, int y, String title, float usage, String info, uint16_t color) {
  int cardWidth = 96;
  int cardHeight = 90;
  
  // 卡片背景
  tft.fillRoundRect(x, y, cardWidth, cardHeight, 8, COLOR_WIDGET);
  
  // 標題
  tft.setTextColor(COLOR_MUTED);
  tft.setTextSize(1);
  tft.setCursor(x + 8, y + 8);
  tft.print(title);
  
  // 連線狀態指示
  if (systemData.connected) {
    tft.fillCircle(x + cardWidth - 12, y + 12, 3, 0x07E0); // 綠色圓點
  } else {
    tft.fillCircle(x + cardWidth - 12, y + 12, 3, 0xF800); // 紅色圓點
  }
  
  if (systemData.connected) {
    // 使用率百分比
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    tft.setCursor(x + 8, y + 25);
    tft.print(String(usage, 1));
    tft.print("%");
    
    // 進度條
    int barWidth = cardWidth - 16;
    int barHeight = 6;
    int barX = x + 8;
    int barY = y + 50;
    
    // 進度條背景
    tft.fillRoundRect(barX, barY, barWidth, barHeight, 3, COLOR_BG);
    
    // 進度條填充
    int fillWidth = (usage / 100.0) * barWidth;
    if (fillWidth > 0) {
      tft.fillRoundRect(barX, barY, fillWidth, barHeight, 3, color);
    }
    
    // 額外資訊（溫度或記憶體）
    tft.setTextColor(COLOR_MUTED);
    tft.setTextSize(1);
    tft.setCursor(x + 8, y + 65);
    if (info.length() > 12) {
      tft.print(info.substring(0, 12));
    } else {
      tft.print(info);
    }
  } else {
    // 斷線狀態
    tft.setTextColor(COLOR_MUTED);
    tft.setTextSize(1);
    tft.setCursor(x + 8, y + 35);
    tft.print("未連線");
  }
}