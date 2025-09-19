# ESP32 桌面信息牌詳細教程（完整版）

## 📚 項目概述

ESP32 桌面信息牌是一個採用 iOS Widgets 風格設計的智能桌面設備，提供多功能信息顯示：

### 🌟 核心功能
- **實時天氣顯示** - 當前天氣狀況、溫度、濕度、風力
- **天氣預報** - 三日天氣預報
- **氣象預警** - 即時氣象警報信息
- **系統監控** - Windows 電腦 CPU/GPU/RAM 使用率監控
- **自訂背景** - 支援上傳自訂背景圖片與毛玻璃效果
- **無線配置** - WiFi AP 模式進行初始設定

### 🎨 設計特色
- iOS Widgets 風格界面設計
- 320×240 TFT 顯示屏適配
- 毛玻璃效果視覺美學
- 藍牙無線數據傳輸
- Web 介面簡單配置

---

## 🛠️ 硬體需求與連接

### 必要硬體
| 項目 | 規格 | 備註 |
|------|------|------|
| 主控板 | ESP32 開發板 | ESP32-DevKitC 或同等 |
| 顯示屏 | 2.8" ILI9341 TFT | 320×240 SPI 接口 |
| 電源 | USB 5V | 透過 ESP32 供電 |

### 接線配置
```
ESP32 GPIO → TFT 接腳
─────────────────────
  18    →   SCK
  19    →   MISO (SDO)
  23    →   MOSI (SDI)
   5    →   CS
   2    →   DC
   4    →   RST
 3.3V   →   VCC
 GND    →   GND
```

### 接線圖示意
```
ESP32                    ILI9341 TFT
┌─────────────┐         ┌─────────────┐
│ GPIO 18 SCK │────────▶│ SCK         │
│ GPIO 19 MISO│◀────────│ SDO (MISO)  │
│ GPIO 23 MOSI│────────▶│ SDI (MOSI)  │
│ GPIO 5  CS  │────────▶│ CS          │
│ GPIO 2  DC  │────────▶│ DC          │
│ GPIO 4  RST │────────▶│ RESET       │
│ 3.3V        │────────▶│ VCC         │
│ GND         │────────▶│ GND         │
└─────────────┘         └─────────────┘
```

---

## 💻 開發環境設置

### 1. 安裝 PlatformIO IDE

#### 方法一：VS Code 擴展（推薦）
1. 安裝 [Visual Studio Code](https://code.visualstudio.com/)
2. 在擴展商店搜尋「PlatformIO IDE」
3. 點擊安裝並等待完成
4. 重啟 VS Code

#### 方法二：命令列安裝
```bash
# 安裝 Python 3.7+
python -m pip install platformio

# 驗證安裝
pio --version
```

### 2. 下載專案代碼
```bash
git clone https://github.com/Howard4995/ESP32-.git
cd ESP32-
```

### 3. 項目結構說明
```
ESP32-/
├── platformio.ini              # PlatformIO 配置文件
├── src/
│   └── ESP32InfoBoard_TFT.ino  # 主程式代碼
├── include/
│   └── icons_tft.h             # 天氣圖標繪製庫
├── windows_monitor/            # Windows 監控程式
│   ├── system_monitor.py       # 主監控程式
│   ├── install.bat             # 自動安裝腳本
│   └── requirements.txt        # Python 依賴
├── preview/                    # 網頁預覽
│   ├── index.html              # 預覽頁面
│   └── widget.css              # 樣式文件
├── README.md                   # 項目說明
├── PROJECT_STRUCTURE.md        # 架構文檔
├── BACKGROUND_GUIDE.md         # 背景自訂指南
└── DETAILED_TUTORIAL.md        # 本教程文件
```

---

## 🔧 編譯與燒錄

### 1. 使用 PlatformIO IDE
1. 開啟 VS Code
2. 選擇 `File` → `Open Folder` → 選擇 ESP32- 專案資料夾
3. 等待 PlatformIO 自動安裝依賴庫
4. 連接 ESP32 開發板到電腦
5. 點擊底部狀態列的「Upload」按鈕（→）
6. 等待編譯並上傳完成

### 2. 使用命令列
```bash
cd ESP32-
pio run --target upload
```

### 3. 監控序列輸出
```bash
pio device monitor --baud 115200
```

### 依賴庫自動安裝
項目會自動安裝以下庫：
- `Adafruit GFX Library` - 圖形繪製基礎
- `Adafruit ILI9341` - TFT 顯示驅動
- `ArduinoJson` - JSON 數據處理

---

## ⚙️ 初始設定配置

### 1. 首次啟動流程

#### 步驟 1：開機檢測
ESP32 上電後會執行以下檢測：
```
啟動畫面 → WiFi 設定檢查 → AP 模式/正常模式
```

#### 步驟 2：進入 AP 模式
如果沒有 WiFi 設定，ESP32 會自動進入 AP 模式：
- **網路名稱（SSID）**：`ESP32-InfoBoard-Setup`
- **網路密碼**：`12345678`
- **管理頁面**：`http://192.168.4.1`

#### 步驟 3：連接配置網路
1. 用手機或電腦連接 `ESP32-InfoBoard-Setup` WiFi
2. 瀏覽器開啟 `http://192.168.4.1`
3. 進入配置介面

### 2. Web 配置介面

#### 基本設定區段
```html
┌─────────────────────────────────┐
│ ESP32 桌面信息牌                 │
├─────────────────────────────────┤
│ 基本設定                        │
│ ┌─────────────────────────────┐ │
│ │ WiFi SSID: [____________] │ │
│ │ WiFi 密碼: [____________] │ │
│ │ API Key:   [____________] │ │
│ │ 城市名稱:  [____________] │ │
│ │ 背景樣式:  [▼ 選擇樣式   ] │ │
│ └─────────────────────────────┘ │
│ [保存設定並重啟]                 │
└─────────────────────────────────┘
```

#### 設定參數說明
| 參數 | 說明 | 範例 |
|------|------|------|
| WiFi SSID | 家中或辦公室 WiFi 名稱 | `MyHome_WiFi` |
| WiFi 密碼 | WiFi 連接密碼 | `mypassword123` |
| API Key | 和風天氣 API 金鑰 | `abcd1234...` |
| 城市名稱 | 天氣查詢城市 | `台北` / `Beijing` |
| 背景樣式 | 原始樣式或自訂背景 | `原始樣式` / `自訂圖片背景` |

### 3. 取得和風天氣 API Key

#### 註冊步驟
1. 前往 [和風天氣開發者平台](https://dev.qweather.com)
2. 點擊「註冊」建立帳戶
3. 登入後進入「控制台」
4. 建立新專案：
   - 專案名稱：`ESP32 信息牌`
   - 專案類型：選擇「Web API」
   - 訂閱計劃：選擇「免費訂閱」
5. 複製 API Key 到配置介面

#### API Key 使用限制
- **免費版本**：每日 1000 次請求
- **ESP32 用量**：約每日 144 次（每 10 分鐘更新）
- **足夠使用**：遠低於限制額度

### 4. 城市名稱設定
支援多種格式：
- **中文**：`台北`、`上海`、`廣州`
- **英文**：`Taipei`、`Shanghai`、`Guangzhou` 
- **拼音**：`taibei`、`shanghai`、`guangzhou`

系統會自動查詢城市 ID，取得第一筆符合結果。

---

## 🎨 背景自訂功能

### 1. 背景樣式選擇

#### 原始樣式
- 深藍灰色背景 (#0861)
- 深灰色資訊區塊 (#2124)
- 經典 iOS Widgets 風格

#### 自訂圖片背景
- 使用者上傳的背景圖片
- 毛玻璃效果資訊區塊
- 半透明白色覆蓋層
- 增強視覺美感

### 2. 背景圖片要求

#### 技術規格
| 項目 | 建議值 | 說明 |
|------|--------|------|
| 尺寸 | 320×240 像素 | 與螢幕解析度匹配 |
| 格式 | JPG, PNG | 常見圖片格式 |
| 檔案大小 | < 1MB | ESP32 儲存空間限制 |
| 色彩 | RGB 全彩 | 16 位元色彩深度 |

#### 圖片建議
- **風景照片**：自然美景、城市夜景
- **抽象圖案**：幾何圖形、漸層背景
- **低對比度**：避免干擾文字閱讀
- **中等亮度**：確保毛玻璃效果明顯

### 3. 上傳背景流程

#### 操作步驟
1. 在配置頁面選擇「自訂圖片背景」
2. 點擊「選擇圖片檔案」
3. 選擇本地圖片文件
4. 預覽圖片效果
5. 點擊「上傳背景圖片」
6. 等待上傳完成
7. 保存設定並重啟

#### 技術實現
背景圖片會被：
1. 轉換為 16 位元 RGB565 格式
2. 儲存到 ESP32 LittleFS 檔案系統
3. 開機時自動載入顯示
4. 配合毛玻璃效果渲染

### 4. 毛玻璃效果

#### 視覺特效
- **半透明背景**：rgba(255,255,255,0.15)
- **模糊邊緣**：模擬玻璃材質
- **細微邊框**：增強立體感
- **文字對比**：自動調整可讀性

#### 實現技術
```cpp
// 毛玻璃效果顏色定義
#define COLOR_GLASS   0x39E7    // 半透明白色

// 繪製毛玻璃區塊
void drawGlassWidget(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius) {
  // 半透明填充
  tft.fillRoundRect(x, y, w, h, radius, COLOR_GLASS);
  // 邊框效果
  tft.drawRoundRect(x, y, w, h, radius, COLOR_TEXT);
}
```

---

## 📱 Windows 系統監控

### 1. 功能概述

Windows 系統監控程式透過藍牙將電腦系統資訊傳送到 ESP32 顯示：

#### 監控項目
- **CPU 使用率**：處理器負載百分比
- **GPU 使用率**：顯卡使用狀況  
- **RAM 使用率**：記憶體使用量
- **CPU 溫度**：處理器溫度（如支援）
- **GPU 溫度**：顯卡溫度（如支援）

#### 傳輸特性
- **無線連接**：藍牙 SPP 協定
- **即時更新**：每 2 秒更新一次
- **自動重連**：斷線自動嘗試重連
- **開機啟動**：支援 Windows 開機自啟動

### 2. 系統需求

#### Windows 系統
- **作業系統**：Windows 10/11
- **Python 版本**：3.7 或更新
- **藍牙功能**：內建或外接藍牙
- **硬碟空間**：約 50MB

#### ESP32 需求
- 已燒錄最新固件
- 藍牙功能正常
- 電源供應穩定

### 3. 安裝與設定

#### 步驟 1：下載監控程式
```bash
# 複製整個 windows_monitor 資料夾到本地
cp -r windows_monitor/ C:\ESP32_Monitor\
cd C:\ESP32_Monitor\
```

#### 步驟 2：自動安裝
1. 雙擊執行 `install.bat`
2. 腳本會自動：
   - 檢查 Python 環境
   - 安裝必要套件
   - 建立啟動腳本
   - 建立自啟動腳本

#### 步驟 3：藍牙配對
1. 確保 ESP32 已開機（顯示 "藍牙已啟動，等待連線..."）
2. 開啟 Windows 藍牙設定
3. 搜尋新裝置，找到 `ESP32-InfoBoard`
4. 點擊配對，無需輸入密碼
5. 記錄配對成功後的 COM 埠號

#### 步驟 4：啟動監控
```batch
# 手動啟動
雙擊 start_monitor.bat

# 開機自啟動設定
雙擊 autostart_setup.bat（需要管理員權限）
```

### 4. 監控資料格式

#### JSON 傳輸格式
```json
{
  "type": "system",
  "timestamp": "2024-01-01T12:00:00",
  "cpu_usage": 45.2,
  "gpu_usage": 78.5, 
  "ram_usage": 67.3,
  "ram_total": 16.0,
  "ram_used": 10.8,
  "cpu_temp": "65°C",
  "gpu_temp": "72°C"
}
```

#### 資料說明
| 欄位 | 類型 | 說明 | 範例 |
|------|------|------|------|
| type | String | 資料類型標識 | "system" |
| timestamp | String | 時間戳記 | "2024-01-01T12:00:00" |
| cpu_usage | Float | CPU 使用率 (%) | 45.2 |
| gpu_usage | Float | GPU 使用率 (%) | 78.5 |
| ram_usage | Float | RAM 使用率 (%) | 67.3 |
| ram_total | Float | 總記憶體 (GB) | 16.0 |
| ram_used | Float | 已用記憶體 (GB) | 10.8 |
| cpu_temp | String | CPU 溫度 | "65°C" |
| gpu_temp | String | GPU 溫度 | "72°C" |

### 5. ESP32 顯示效果

#### 系統監控卡片布局
```
┌─────────────────────────────────────────────┐
│ [●] CPU        [●] GPU        [●] RAM       │
│     45%            78%           67%        │
│ ████████░░     ████████████   ████████░     │
│ 8 核心          RTX 3070       10.8/16GB   │
└─────────────────────────────────────────────┘
```

#### 狀態指示燈
- **綠色圓點 (●)**：藍牙已連線，資料正常
- **紅色圓點 (●)**：藍牙斷線或無資料

#### 進度條顏色
- **CPU 使用率**：藍色漸層
- **GPU 使用率**：綠色漸層  
- **RAM 使用率**：橙色漸層

---

## 🌐 Web API 介面

### 1. API 端點概覽

ESP32 提供多個 REST API 端點用於資料查詢和系統控制：

#### 基礎路由
| 端點 | 方法 | 功能 | 回應格式 |
|------|------|------|----------|
| `/` | GET | 配置頁面 | HTML |
| `/api/save` | POST | 儲存設定 | JSON |
| `/api/weather` | GET | 天氣資料 | JSON |
| `/api/upload` | POST | 上傳背景 | JSON |
| `/api/test-image` | GET | 測試背景 | JSON |

### 2. 天氣 API 詳細說明

#### `/api/weather` 回應格式
```json
{
  "current": {
    "temp": "28",
    "text": "多雲",
    "humidity": "65",
    "windDir": "東",
    "windScale": "3",
    "icon": 1,
    "lastUpdate": 1704038400000
  },
  "forecast": [
    {
      "date": "09-18",
      "tempMax": "32",
      "tempMin": "25", 
      "icon": 1
    },
    {
      "date": "09-19",
      "tempMax": "30",
      "tempMin": "24",
      "icon": 2
    },
    {
      "date": "09-20", 
      "tempMax": "31",
      "tempMin": "26",
      "icon": 0
    }
  ],
  "warning": {
    "text": "高溫預警",
    "level": "黃色",
    "active": true,
    "lastUpdate": 1704038400000
  },
  "system": {
    "cpuUsage": 45.2,
    "gpuUsage": 78.5,
    "ramUsage": 67.3,
    "connected": true,
    "lastUpdate": 1704038400000
  }
}
```

### 3. 圖標代碼對應

#### IconType 枚舉
```cpp
enum IconType {
  ICON_UNKNOWN = 0,  // 未知天氣
  ICON_SUNNY,        // 晴天 ☀️
  ICON_CLOUDY,       // 多雲 ⛅
  ICON_RAIN,         // 雨天 🌧️
  ICON_SNOW,         // 雪天 ❄️
  ICON_FOG,          // 霧天 🌫️
  ICON_THUNDER       // 雷雨 ⛈️
};
```

### 4. 設定儲存 API

#### `/api/save` 請求參數
```javascript
// POST 請求參數
{
  "wifi_ssid": "MyHome_WiFi",
  "wifi_password": "mypassword123", 
  "qweather_key": "abcd1234...",
  "city_name": "台北",
  "background_style": "custom"
}
```

#### 成功回應
```json
{
  "status": "success"
}
```

設定儲存後，ESP32 會自動重啟並應用新設定。

---

## 🎯 界面顯示說明

### 1. 整體布局

#### 螢幕區域劃分（320×240）
```
 0 ┌─────────────────────────────────────┐
   │ 狀態列 (0-24px)                      │ 24
26 ├─────────────────────────────────────┤
   │ 時鐘卡片      │ 現在天氣卡片         │
   │ (26-140px)    │ (26-140px)          │ 140
142├─────────────────────────────────────┤
   │ 系統監控卡片 (142-232px)             │ 232
235├─────────────────────────────────────┤
   │ 預警資訊條 (235-257px)               │ 257
   └─────────────────────────────────────┘
   0                                   320
```

### 2. 狀態列 (Status Bar)

#### 顯示內容
- **左側**：城市名稱與國家
- **右側**：IP 位址

#### 樣式設計
```cpp
// 狀態列背景
tft.fillRect(0, 0, 320, 24, COLOR_BG);

// 城市資訊晶片
tft.fillRoundRect(8, 4, cityWidth, 16, 8, COLOR_WIDGET);
tft.setTextColor(COLOR_TEXT);
tft.print("台北市 台灣");

// IP 位址晶片  
tft.fillRoundRect(320-ipWidth-8, 4, ipWidth, 16, 8, COLOR_WIDGET);
tft.print("IP 192.168.1.88");
```

### 3. 時鐘卡片

#### 顯示內容
- **大時間**：HH:MM 格式
- **日期**：MM-DD Week 格式

#### 範例顯示
```
┌─────────────────┐
│ 時鐘             │
│                 │
│    22:38        │
│                 │
│  09-18 Thu      │
└─────────────────┘
```

### 4. 現在天氣卡片

#### 布局設計
```
┌─────────────────┐
│ 現在天氣         │
│ ☀️         28°  │
│                 │
│ 多雲             │
│ 濕度 65% · 東風 3級│
└─────────────────┘
```

#### 文字縮略策略
- 確保兩行文字在 108px 寬度內完整顯示
- 自動調整字體大小
- 風向風力資訊合併顯示

### 5. 系統監控卡片

#### 三欄式布局
```
┌─────────────────────────────────────────┐
│ [●] CPU      [●] GPU      [●] RAM       │
│     45%          78%          67%       │
│ ████████░░   ██████████   ████████░     │
│ 8 核心        RTX 3070     10.8/16GB   │
└─────────────────────────────────────────┘
```

#### 進度條繪製
```cpp
// 計算進度條長度
int barWidth = (usage / 100.0) * maxBarWidth;

// 繪製背景
tft.fillRect(x, y, maxBarWidth, barHeight, COLOR_MUTED);

// 繪製進度
tft.fillRect(x, y, barWidth, barHeight, progressColor);
```

### 6. 預警資訊條

#### 顯示規則
- **有預警**：顯示預警級別與內容
- **無預警**：隱藏或顯示「無預警」

#### 顏色編碼
- **藍色**：一般提醒
- **黃色**：注意級別
- **橙色**：警告級別
- **紅色**：嚴重警告

---

## 🔧 故障排除

### 1. 編譯錯誤

#### 常見錯誤與解決方案

**錯誤：`fatal error: WiFi.h: No such file or directory`**
```bash
# 解決方案：確認 ESP32 開發板已安裝
pio platform install espressif32
```

**錯誤：`Library 'Adafruit GFX Library' not found`**
```bash
# 解決方案：手動安裝庫
pio lib install "adafruit/Adafruit GFX Library"
pio lib install "adafruit/Adafruit ILI9341"
pio lib install "bblanchon/ArduinoJson"
```

**錯誤：`A fatal error occurred: Failed to connect to ESP32`**
- 檢查 USB 連接線
- 確認 ESP32 是否進入下載模式
- 嘗試按住 BOOT 按鈕後按 RESET

### 2. WiFi 連接問題

#### 診斷步驟
1. **檢查序列輸出**：
```bash
pio device monitor --baud 115200
```

2. **確認 WiFi 設定**：
   - SSID 拼寫正確
   - 密碼無誤
   - 網路支援 2.4GHz（ESP32 不支援 5GHz）

3. **重設 WiFi 設定**：
```cpp
// 在 setup() 中加入
prefs.clear();  // 清除所有設定
```

#### 常見問題
- **訊號強度不足**：移近路由器
- **密碼錯誤**：重新輸入密碼
- **MAC 位址過濾**：路由器允許 ESP32 MAC

### 3. 天氣資料問題

#### API Key 相關
**問題：天氣資料無法更新**
- 檢查 API Key 有效性
- 確認 API 配額未超限
- 驗證城市名稱正確

**問題：城市 ID 查詢失敗**
- 嘗試不同城市名稱格式
- 確認網路連接正常
- 檢查和風天氣服務狀態

#### HTTPS 連接問題
```cpp
// 序列輸出檢查
if (client.connect("devapi.qweather.com", 443)) {
  Serial.println("HTTPS 連接成功");
} else {
  Serial.println("HTTPS 連接失敗");
}
```

### 4. 顯示屏問題

#### 黑屏或花屏
1. **檢查接線**：
   - 確認所有接線正確
   - 檢查接觸是否良好
   - 驗證電源供應

2. **檢查程式碼**：
```cpp
void setup() {
  // 確認這些初始化代碼存在
  tft.begin();
  tft.setRotation(1);  // 必須設為 1
  tft.fillScreen(COLOR_BG);
}
```

3. **測試顯示**：
```cpp
// 簡單測試程式
tft.fillScreen(ILI9341_RED);
delay(1000);
tft.fillScreen(ILI9341_GREEN);
delay(1000);
tft.fillScreen(ILI9341_BLUE);
```

#### 顯示方向錯誤
```cpp
// 確保使用正確旋轉
tft.setRotation(1);  // 橫向 320x240
```

### 5. 藍牙連接問題

#### ESP32 端診斷
```cpp
void setup() {
  // 檢查藍牙初始化
  if (!SerialBT.begin("ESP32-InfoBoard")) {
    Serial.println("藍牙初始化失敗");
  } else {
    Serial.println("藍牙已啟動，裝置名稱: ESP32-InfoBoard");
  }
}
```

#### Windows 端診斷
1. **檢查 COM 埠**：
```python
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
for port in ports:
    print(f"Port: {port.device}, Description: {port.description}")
```

2. **手動測試連接**：
```python
import serial
try:
    ser = serial.Serial('COM3', 115200)  # 替換為實際 COM 埠
    print("連接成功")
    ser.close()
except Exception as e:
    print(f"連接失敗: {e}")
```

#### 常見藍牙問題
- **配對失敗**：重新配對裝置
- **COM 埠錯誤**：檢查裝置管理員中的埠號
- **權限問題**：以管理員身份執行監控程式

### 6. 自訂背景問題

#### 圖片無法顯示
1. **檢查檔案格式**：
   - 確保是 JPG 或 PNG 格式
   - 檔案大小不超過 1MB
   - 尺寸建議 320×240

2. **檢查 LittleFS**：
```cpp
void setup() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS 初始化失敗");
  } else {
    Serial.println("LittleFS 初始化成功");
    // 列出檔案
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while(file) {
      Serial.println(file.name());
      file = root.openNextFile();
    }
  }
}
```

#### 毛玻璃效果異常
- 確認背景樣式設為 "custom"
- 檢查 COLOR_GLASS 顏色定義
- 驗證 drawGlassWidget() 函數

---

## 📊 效能優化建議

### 1. 顯示效能優化

#### 減少不必要的重繪
```cpp
void updateTimeDisplay() {
  static String lastTime = "";
  String currentTime = getCurrentTime();
  
  // 只在時間改變時更新
  if (currentTime != lastTime) {
    drawTimeOnly();
    lastTime = currentTime;
  }
}
```

#### 使用區域更新
```cpp
// 只更新改變的區域，而非整個螢幕
void updateTemperature(String newTemp) {
  tft.fillRect(tempX, tempY, tempWidth, tempHeight, COLOR_WIDGET);
  tft.setTextColor(COLOR_TEXT);
  tft.print(newTemp);
}
```

### 2. 記憶體管理

#### 避免字串記憶體碎片
```cpp
// 使用 String.reserve() 預分配記憶體
String jsonBuffer;
jsonBuffer.reserve(1024);  // 預分配 1KB
```

#### 監控記憶體使用
```cpp
void printMemoryInfo() {
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Free PSRAM: ");
  Serial.println(ESP.getFreePsram());
}
```

### 3. 網路請求優化

#### 合理的更新頻率
```cpp
// 天氣資料：10 分鐘更新一次
const unsigned long WEATHER_UPDATE_INTERVAL = 600000;

// 系統監控：2 秒更新一次  
const unsigned long SYSTEM_UPDATE_INTERVAL = 2000;

// 預警資訊：30 分鐘更新一次
const unsigned long WARNING_UPDATE_INTERVAL = 1800000;
```

#### 錯誤重試機制
```cpp
void updateWeatherWithRetry() {
  int retryCount = 0;
  while (retryCount < 3) {
    if (updateWeather()) {
      Serial.println("天氣更新成功");
      return;
    }
    retryCount++;
    delay(5000);  // 等待 5 秒後重試
  }
  Serial.println("天氣更新失敗，已重試 3 次");
}
```

---

## 🚀 進階功能擴展

### 1. 新增天氣項目

#### 添加 UV 指數顯示
```cpp
struct WeatherData {
  // 現有欄位...
  String uvIndex = "--";      // 新增 UV 指數
  String uvLevel = "低";      // UV 等級
};

void updateWeather() {
  // 在 JSON 解析中添加
  if (doc["now"]["uv"]) {
    currentWeather.uvIndex = doc["now"]["uv"].as<String>();
  }
}
```

#### 顯示空氣品質指數
```cpp
// 新增 AQI 資料結構
struct AirQualityData {
  String aqi = "--";
  String level = "良好";
  String pm25 = "--";
  String pm10 = "--";
};

// 新增 API 端點處理
void updateAirQuality() {
  String url = "/v7/air/now?location=" + city_id + "&key=" + qweather_key;
  // 實現 AQI 資料請求...
}
```

### 2. 多城市天氣支援

#### 資料結構調整
```cpp
struct CityWeather {
  String cityName;
  String cityId;
  WeatherData weather;
};

CityWeather cities[3];  // 支援 3 個城市
int currentCityIndex = 0;
```

#### 城市切換邏輯
```cpp
void switchCity() {
  currentCityIndex = (currentCityIndex + 1) % 3;
  updateWeatherDisplay(cities[currentCityIndex]);
}

// 每 30 秒自動切換城市
void handleCitySwitching() {
  static unsigned long lastSwitch = 0;
  if (millis() - lastSwitch > 30000) {
    switchCity();
    lastSwitch = millis();
  }
}
```

### 3. 智慧通知功能

#### 天氣預警推播
```cpp
void checkWeatherAlerts() {
  if (warning.active && !lastAlertShown) {
    // 閃爍預警條 3 次
    for (int i = 0; i < 3; i++) {
      tft.fillRect(0, 235, 320, 22, COLOR_ORANGE);
      delay(500);
      drawWarningBar();
      delay(500);
    }
    lastAlertShown = true;
  }
}
```

#### 系統效能警告
```cpp
void checkSystemAlerts() {
  if (systemData.cpuUsage > 90) {
    // CPU 過載警告
    showSystemAlert("CPU 使用率過高", COLOR_ORANGE);
  }
  if (systemData.ramUsage > 85) {
    // 記憶體不足警告
    showSystemAlert("記憶體使用率過高", COLOR_ORANGE);
  }
}
```

### 4. 觸控互動功能

#### 添加觸控螢幕支援
```cpp
#include <XPT2046_Touchscreen.h>

#define TOUCH_CS 21
XPT2046_Touchscreen touch(TOUCH_CS);

void setup() {
  touch.begin();
  // 其他初始化...
}

void handleTouch() {
  if (touch.touched()) {
    TS_Point point = touch.getPoint();
    
    // 觸控區域判斷
    if (point.x > 160 && point.y < 140) {
      // 點擊天氣卡片，切換顯示模式
      toggleWeatherDisplay();
    }
  }
}
```

### 5. 資料記錄功能

#### SD 卡資料紀錄
```cpp
#include <SD.h>

void saveDataToSD() {
  File dataFile = SD.open("weather_log.csv", FILE_APPEND);
  if (dataFile) {
    String logEntry = getCurrentTime() + "," + 
                     currentWeather.temp + "," +
                     currentWeather.humidity + "," +
                     String(systemData.cpuUsage);
    dataFile.println(logEntry);
    dataFile.close();
  }
}
```

#### 資料統計分析
```cpp
struct DailyStats {
  float avgTemp;
  float maxTemp;
  float minTemp;
  float avgCpuUsage;
  float maxCpuUsage;
};

DailyStats calculateDailyStats() {
  // 從記錄檔案中計算統計資料
  // 返回統計結果
}
```

---

## 📋 維護與更新

### 1. 定期維護清單

#### 每週檢查項目
- [ ] 檢查顯示屏是否正常
- [ ] 確認 WiFi 連接穩定
- [ ] 驗證天氣資料更新
- [ ] 檢查系統監控連線
- [ ] 清理序列輸出日誌

#### 每月檢查項目  
- [ ] 檢查 API Key 使用量
- [ ] 更新城市設定（如需要）
- [ ] 檢查 Flash 儲存空間使用
- [ ] 驗證自訂背景功能
- [ ] 檢查 Windows 監控程式運行狀況

#### 每季檢查項目
- [ ] 更新 ESP32 韌體
- [ ] 升級相依庫版本
- [ ] 檢查硬體連接
- [ ] 備份設定檔案
- [ ] 清理暫存檔案

### 2. 軟體更新流程

#### 韌體更新步驟
1. **備份現有設定**：
```cpp
void backupSettings() {
  File file = LittleFS.open("/backup_config.json", "w");
  JsonDocument doc;
  doc["wifi_ssid"] = wifi_ssid;
  doc["wifi_password"] = wifi_password;
  doc["qweather_key"] = qweather_key;
  doc["city_name"] = city_name;
  serializeJson(doc, file);
  file.close();
}
```

2. **下載新版本韌體**
3. **上傳新韌體到 ESP32**
4. **恢復設定檔案**
5. **驗證功能正常**

#### 庫版本更新
```bash
# 更新所有庫到最新版本
pio lib update

# 更新特定庫
pio lib update "Adafruit GFX Library"
```

### 3. 故障預防措施

#### 電源保護
- 使用穩定的 USB 電源供應器
- 添加電容濾波減少電源噪音
- 避免頻繁斷電重啟

#### 環境保護
- 避免高溫高濕環境
- 防止灰塵積累在螢幕上
- 避免陽光直射

#### 軟體保護
```cpp
// 添加看門狗定時器
#include <esp_task_wdt.h>

void setup() {
  // 啟用看門狗定時器 (10 秒)
  esp_task_wdt_init(10, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  // 餵狗操作
  esp_task_wdt_reset();
  // 其他程式邏輯...
}
```

---

## 🎉 總結

ESP32 桌面信息牌是一個功能豐富的 IoT 項目，結合了：

### ✅ 已實現功能
- **天氣資訊顯示** - 即時天氣、預報、預警
- **系統監控** - Windows 電腦 CPU/GPU/RAM 監控
- **自訂背景** - 圖片上傳與毛玻璃效果
- **無線配置** - WiFi AP 模式設定介面
- **Web API** - RESTful API 支援
- **藍牙通訊** - 無線資料傳輸

### 🔧 技術特色
- **iOS Widgets 風格** - 現代化界面設計
- **模組化程式架構** - 易於維護和擴展
- **多平台支援** - ESP32 + Windows 整合
- **即時資料更新** - 自動化資料同步
- **錯誤處理機制** - 穩定可靠運行

### 🚀 擴展可能性
- 多城市天氣支援
- 觸控互動功能
- 資料記錄分析
- 智慧通知推播
- 更多系統監控項目

這個專案展示了 ESP32 在 IoT 應用中的強大潛力，通過整合多種技術實現了一個實用且美觀的桌面信息終端。

---

## 📞 技術支援

如果在使用過程中遇到問題，可以：

1. **查看本教程** - 詳細的故障排除章節
2. **檢查序列輸出** - `pio device monitor --baud 115200`  
3. **查看項目 README** - 基本使用說明
4. **參考程式碼註解** - 詳細的實現說明

祝您使用愉快！🎯