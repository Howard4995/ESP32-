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
#include <FS.h>
#include <LittleFS.h>
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
#define COLOR_GLASS   0x39E7    // 半透明白色 (毛玻璃效果)

// WiFi AP 設定
const char* AP_SSID = "ESP32-InfoBoard-Setup";
const char* AP_PASSWORD = "12345678";

// 全域變數
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
WebServer server(80);
Preferences prefs;

String wifi_ssid = "";
String wifi_password = "";
String qweather_key = "";
String city_name = "";
String city_id = "";
String background_style = "original";  // "original" or "custom"
bool hasCustomBackground = false;

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

WeatherData currentWeather;
ForecastData forecast[3];
WarningData warning;

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
  
  // 初始化檔案系統
  if (!LittleFS.begin()) {
    Serial.println("LittleFS 初始化失敗");
  } else {
    Serial.println("LittleFS 初始化成功");
    checkCustomBackground();
  }
  
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
  
  unsigned long now = millis();
  
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
  background_style = prefs.getString("bg_style", "original");
  
  Serial.println("設定載入完成");
}

void saveSettings() {
  prefs.putString("wifi_ssid", wifi_ssid);
  prefs.putString("wifi_pwd", wifi_password);
  prefs.putString("qw_key", qweather_key);
  prefs.putString("city_name", city_name);
  prefs.putString("city_id", city_id);
  prefs.putString("bg_style", background_style);
  
  Serial.println("設定已儲存");
}

void checkCustomBackground() {
  if (LittleFS.exists("/background.raw")) {
    hasCustomBackground = true;
    Serial.println("發現自訂背景圖片");
  } else {
    hasCustomBackground = false;
    Serial.println("未發現自訂背景圖片");
  }
}

void drawBackground() {
  if (background_style == "custom" && hasCustomBackground) {
    drawCustomBackground();
  } else {
    tft.fillScreen(COLOR_BG);
  }
}

void drawCustomBackground() {
  if (!hasCustomBackground) return;
  
  File file = LittleFS.open("/background.raw", "r");
  if (!file) {
    Serial.println("無法開啟背景圖片檔案");
    tft.fillScreen(COLOR_BG);
    return;
  }
  
  // 讀取並顯示 320x240 的 16-bit 圖像資料
  uint16_t pixel;
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 320; x++) {
      if (file.available() >= 2) {
        file.read((uint8_t*)&pixel, 2);
        tft.drawPixel(x, y, pixel);
      }
    }
  }
  file.close();
}

void drawGlassWidget(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius) {
  if (background_style == "custom" && hasCustomBackground) {
    // 毛玻璃效果：半透明白色背景
    tft.fillRoundRect(x, y, w, h, radius, COLOR_GLASS);
    // 添加邊框增強玻璃效果
    tft.drawRoundRect(x, y, w, h, radius, 0xBDF7);
  } else {
    // 原始樣式
    tft.fillRoundRect(x, y, w, h, radius, COLOR_WIDGET);
  }
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
    if (server.hasArg("background_style")) {
      background_style = server.arg("background_style");
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
  
  // 圖片上傳處理
  server.on("/api/upload", HTTP_POST, []() {
    server.send(200, "application/json", "{\"status\":\"success\"}");
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.println("開始接收背景圖片...");
      File file = LittleFS.open("/background.raw", "w");
      if (!file) {
        Serial.println("無法建立背景圖片檔案");
        return;
      }
      file.close();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      File file = LittleFS.open("/background.raw", "a");
      if (file) {
        file.write(upload.buf, upload.currentSize);
        file.close();
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      Serial.println("背景圖片上傳完成");
      hasCustomBackground = true;
    }
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
        h2 { color: #666; border-bottom: 1px solid #ddd; padding-bottom: 5px; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="password"], input[type="file"], select { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background: #007AFF; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; margin-top: 10px; }
        button:hover { background: #0056CC; }
        .info { background: #e7f3ff; padding: 10px; border-radius: 4px; margin-bottom: 20px; font-size: 14px; }
        .upload-section { background: #f8f9fa; padding: 15px; border-radius: 8px; margin-top: 20px; }
        #backgroundPreview { display: none; margin-top: 10px; }
        #backgroundPreview img { max-width: 100%; border-radius: 4px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 桌面信息牌</h1>
        <div class="info">
            <strong>設定說明：</strong><br>
            • WiFi：設定網路連線<br>
            • API Key：到 <a href="https://dev.qweather.com" target="_blank">和風天氣</a> 免費註冊<br>
            • 城市：可用中文、英文或拼音<br>
            • 背景樣式：原始樣式或自訂圖片背景（毛玻璃效果）
        </div>
        
        <form onsubmit="saveConfig(event)">
            <h2>基本設定</h2>
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
            
            <h2>背景樣式</h2>
            <div class="form-group">
                <label>選擇背景樣式：</label>
                <select name="background_style" onchange="toggleUploadSection()">
                    <option value="original">原始樣式</option>
                    <option value="custom">自訂圖片背景</option>
                </select>
            </div>
            
            <button type="submit">保存設定並重啟</button>
        </form>
        
        <div class="upload-section" id="uploadSection" style="display: none;">
            <h2>背景圖片上傳</h2>
            <div class="info">
                <strong>圖片要求：</strong><br>
                • 建議尺寸：320×240 像素<br>
                • 支援格式：JPG, PNG<br>
                • 檔案大小：&lt; 1MB<br>
                • 上傳後將以毛玻璃效果顯示資訊區塊
            </div>
            <form onsubmit="uploadBackground(event)" enctype="multipart/form-data">
                <div class="form-group">
                    <label>選擇圖片檔案：</label>
                    <input type="file" name="background" accept="image/*" onchange="previewImage(this)" required>
                </div>
                <div id="backgroundPreview">
                    <img id="previewImg" src="" alt="預覽">
                </div>
                <button type="submit">上傳背景圖片</button>
            </form>
        </div>
    </div>
    
    <script>
        function toggleUploadSection() {
            const select = document.querySelector('select[name="background_style"]');
            const uploadSection = document.getElementById('uploadSection');
            if (select.value === 'custom') {
                uploadSection.style.display = 'block';
            } else {
                uploadSection.style.display = 'none';
            }
        }
        
        function previewImage(input) {
            if (input.files && input.files[0]) {
                const reader = new FileReader();
                reader.onload = function(e) {
                    document.getElementById('previewImg').src = e.target.result;
                    document.getElementById('backgroundPreview').style.display = 'block';
                };
                reader.readAsDataURL(input.files[0]);
            }
        }
        
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
        
        function uploadBackground(event) {
            event.preventDefault();
            const form = event.target;
            const formData = new FormData(form);
            
            if (!formData.get('background').name) {
                alert('請選擇圖片檔案');
                return;
            }
            
            // 先轉換圖片為適當格式
            const file = formData.get('background');
            const canvas = document.createElement('canvas');
            const ctx = canvas.getContext('2d');
            const img = new Image();
            
            img.onload = function() {
                canvas.width = 320;
                canvas.height = 240;
                ctx.drawImage(img, 0, 0, 320, 240);
                
                // 轉換為16-bit RGB565格式
                const imageData = ctx.getImageData(0, 0, 320, 240);
                const buffer = new ArrayBuffer(320 * 240 * 2);
                const view = new Uint16Array(buffer);
                
                for (let i = 0; i < imageData.data.length; i += 4) {
                    const r = imageData.data[i] >> 3;
                    const g = imageData.data[i + 1] >> 2;
                    const b = imageData.data[i + 2] >> 3;
                    const rgb565 = (r << 11) | (g << 5) | b;
                    view[i / 4] = rgb565;
                }
                
                const blob = new Blob([buffer], { type: 'application/octet-stream' });
                const uploadData = new FormData();
                uploadData.append('background', blob, 'background.raw');
                
                fetch('/api/upload', {
                    method: 'POST',
                    body: uploadData
                })
                .then(response => response.json())
                .then(data => {
                    alert('背景圖片上傳成功！請重新啟動裝置以套用新背景。');
                })
                .catch(error => {
                    alert('上傳失敗：' + error);
                });
            };
            
            img.src = URL.createObjectURL(file);
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
  drawBackground();
  
  // 繪製狀態列
  drawStatusBar();
  
  // 繪製時鐘卡片
  drawClockCard();
  
  // 繪製天氣卡片
  drawWeatherCard();
  
  // 繪製預報卡片
  drawForecastCards();
  
  // 繪製預警列
  drawWarningBar();
}

void drawStatusBar() {
  // 狀態列背景
  if (background_style == "custom" && hasCustomBackground) {
    tft.fillRect(0, 0, 320, 24, COLOR_GLASS);
  } else {
    tft.fillRect(0, 0, 320, 24, COLOR_WIDGET);
  }
  
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
  drawGlassWidget(8, 32, 182, 102, 12);
  
  // 標題
  tft.setTextColor(COLOR_MUTED);
  tft.setTextSize(1);
  tft.setCursor(16, 42);
  tft.print("時鐘");
  
  updateTimeDisplay();
}

void updateTimeDisplay() {
  // 清除時間區域 - 使用與背景樣式相符的填色
  if (background_style == "custom" && hasCustomBackground) {
    tft.fillRect(16, 55, 166, 70, COLOR_GLASS);
  } else {
    tft.fillRect(16, 55, 166, 70, COLOR_WIDGET);
  }
  
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
  drawGlassWidget(198, 32, 114, 102, 12);
  
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
    drawGlassWidget(x, y, 96, 60, 8);
    
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
    
    tft.fillRoundRect(8, 210, 304, 22, 11, warningColor);
    
    // 預警文字
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(1);
    tft.setCursor(16, 218);
    
    // 截斷過長文字
    String displayText = warning.text;
    if (displayText.length() > 35) {
      displayText = displayText.substring(0, 32) + "...";
    }
    tft.print(displayText);
  } else {
    // 清除預警區域
    tft.fillRect(8, 210, 304, 22, COLOR_BG);
  }
}