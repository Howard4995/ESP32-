# ESP32 桌面信息牌項目架構說明

## 項目概述

這是一個基於 ESP32 的智能桌面信息牌項目，採用 iOS Widgets 風格設計，能夠顯示：
- 實時時間和日期
- 當前天氣狀況  
- 3日天氣預報
- 氣象預警信息

## 項目結構

```
ESP32-/
├── README.md                    # 項目說明文檔
├── platformio.ini              # PlatformIO 建置配置
├── src/
│   └── ESP32InfoBoard_TFT.ino  # 主程式代碼
├── include/
│   └── icons_tft.h             # 天氣圖標繪製庫
└── preview/
    ├── index.html              # HTML 預覽頁面
    └── widget.css              # CSS 樣式文件
```

## 硬體需求

### 主控制器
- ESP32 開發板（ESP32-DevKitC 或同等型號）

### 顯示屏
- 2.8" ILI9341 TFT 顯示屏
- 解析度：320×240 像素
- 接口：SPI

### 接線配置
| 功能 | ESP32 GPIO | TFT 接腳 |
|------|------------|----------|
| SCK  | 18         | SCK      |
| MISO | 19         | SDO      |
| MOSI | 23         | SDI      |
| CS   | 5          | CS       |
| DC   | 2          | DC       |
| RST  | 4          | RESET    |
| VCC  | 3.3V       | VCC      |
| GND  | GND        | GND      |

## 軟體架構

### 核心功能模塊

1. **WiFi 管理**
   - 支援 Station 模式連接 WiFi
   - AP 模式用於初始配置
   - 自動重連機制

2. **Web 配置界面**
   - AP 模式下提供配置頁面
   - 支援 WiFi 設定
   - API Key 配置
   - 城市選擇

3. **天氣數據獲取**
   - 使用和風天氣 API
   - 支援城市 ID 查詢
   - 當前天氣、預報、預警

4. **顯示渲染**
   - TFT 圖形界面
   - iOS Widgets 風格設計
   - 圖標繪製系統

### 依賴庫

- **Adafruit GFX Library** - 圖形繪製基礎庫
- **Adafruit ILI9341** - TFT 顯示驅動
- **ArduinoJson** - JSON 數據解析

## API 接口

### 和風天氣 API 端點

1. **城市查詢**
   ```
   GET https://geoapi.qweather.com/v2/city/lookup
   參數：location={城市名}, key={API_KEY}
   ```

2. **實時天氣**
   ```
   GET https://devapi.qweather.com/v7/weather/now
   參數：location={城市ID}, key={API_KEY}
   ```

3. **3日預報**
   ```
   GET https://devapi.qweather.com/v7/weather/3d
   參數：location={城市ID}, key={API_KEY}
   ```

4. **災害預警**
   ```
   GET https://devapi.qweather.com/v7/warning/now
   參數：location={城市ID}, key={API_KEY}
   ```

### 設備 Web API

- `GET /` - 配置頁面
- `POST /api/save` - 保存配置
- `GET /api/weather` - 獲取天氣數據

## 界面布局

### 屏幕分區（320×240）

```
┌─────────────────────────────────┐ 0px
│        狀態列 (24px)            │ ← 城市名 | IP地址
├─────────────┬───────────────────┤ 24px  
│             │                   │
│   時鐘區     │    天氣狀況區      │ 32px
│  (182×102)  │   (114×102)       │
│             │                   │ ← 時間、日期 | 溫度、圖標、描述
├─────────────┴───────────────────┤ 134px
│        3日預報區 (304×60)        │ 142px
│   [日期]   [日期]   [日期]      │ ← 3個預報卡片
│   [圖標]   [圖標]   [圖標]      │
│  [溫度範圍] [溫度範圍] [溫度範圍]  │
├─────────────────────────────────┤ 202px
│        預警信息區 (304×22)       │ 210px
│         橙色/紅色/黃色警告        │ ← 可選顯示
└─────────────────────────────────┘ 240px
```

### 顏色主題

- **背景色**: `0x0861` (深藍灰)
- **卡片背景**: `0x2124` (深灰)  
- **主文字**: `0xFFFF` (白色)
- **次要文字**: `0x8410` (淺灰)
- **強調色**: `0x051D` (藍色)
- **警告色**: `0xFD20` (橙色)

## 圖標系統

### 天氣圖標類型

```cpp
enum IconType {
  ICON_UNKNOWN = 0,  // 未知天氣
  ICON_SUNNY,        // 晴天 ☀️
  ICON_CLOUDY,       // 多雲 ⛅
  ICON_RAIN,         // 雨天 🌧️
  ICON_SNOW,         // 雪天 ❄️
  ICON_FOG,          // 霧天 🌫️
  ICON_THUNDER       // 雷雨 ⛈️
}
```

### 圖標映射規則

| 和風天氣代碼 | 圖標類型 | 描述 |
|-------------|----------|------|
| 100-103     | SUNNY    | 晴天相關 |
| 104-213     | CLOUDY   | 陰天多雲 |
| 300-318     | RAIN     | 降雨相關 |
| 400-410     | SNOW     | 降雪相關 |
| 500-515     | FOG      | 霧霾相關 |
| 600-610     | THUNDER  | 雷雨相關 |

## 配置說明

### 首次設置流程

1. **上電啟動**
   - 設備檢測 WiFi 配置
   - 若無配置，自動進入 AP 模式

2. **AP 模式配置**
   - SSID: `ESP32-InfoBoard-Setup`
   - 密碼: `12345678`
   - 訪問: `http://192.168.4.1`

3. **填寫配置**
   - WiFi SSID/密碼
   - 和風天氣 API Key ([註冊地址](https://dev.qweather.com))
   - 城市名稱（支援中文/英文/拼音）

4. **保存重啟**
   - 配置自動保存到 Flash
   - 設備重啟並連接 WiFi

### 運行模式

- **數據更新頻率**
  - 天氣數據：每 10 分鐘
  - 預警信息：每 30 分鐘  
  - 時間顯示：每 1 秒

- **網絡異常處理**
  - WiFi 斷線自動重連
  - API 調用失敗重試
  - 顯示最後成功數據

## 開發指南

### 編譯環境

1. 安裝 PlatformIO IDE (VS Code 擴展)
2. 打開項目文件夾
3. 連接 ESP32 開發板
4. 點擊 "Upload" 按鈕

### 調試方法

- 串口監控：115200 bps
- 日誌輸出：Serial.println()
- Web API：`/api/weather` 查看數據

### 自定義修改

1. **修改接腳定義**
   ```cpp
   #define TFT_CS   5   // 片選
   #define TFT_DC   2   // 數據/命令
   #define TFT_RST  4   // 復位
   ```

2. **調整更新頻率**
   ```cpp
   // 天氣更新間隔 (毫秒)
   if (now - lastWeatherUpdate > 600000) // 10分鐘
   ```

3. **修改顏色主題**
   ```cpp
   #define COLOR_BG      0x0861  // 背景色
   #define COLOR_WIDGET  0x2124  // 卡片色
   ```

## 故障排除

### 常見問題

1. **WiFi 連接失敗**
   - 檢查 SSID/密碼正確性
   - 確認信號強度充足
   - 嘗試重啟設備

2. **天氣數據不更新**
   - 驗證 API Key 有效性
   - 檢查城市 ID 是否正確
   - 確認網絡連接穩定

3. **顯示異常**
   - 檢查 TFT 接線
   - 確認供電穩定
   - 驗證 `tft.setRotation(1)` 設置

4. **編譯錯誤**
   - 確認 PlatformIO 環境
   - 檢查依賴庫版本
   - 清理並重新編譯

### 調試技巧

- 啟用串口監控查看日誌
- 使用 `/api/weather` 檢查數據
- 檢查 Flash 使用情況
- 監控內存使用量

## 授權聲明

本項目採用開源授權，允許個人和教育用途自由使用和修改。

---

*最後更新：2024年9月*