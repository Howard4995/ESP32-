# ESP32 桌面信息牌（iOS Widgets 風格）

本套件包含：
- ESP32 固件（PlatformIO 專案）
- Windows 系統監控程式（透過藍牙顯示 CPU/GPU/RAM 使用率）
- 預覽用 HTML + CSS（純排版）
- 已移除 UV 生活指數；「現在天氣」以兩行文字確保全部資訊可見

## 功能特色

### 🌤️ 天氣資訊顯示
- 即時天氣資料（溫度、濕度、風力）
- 三日天氣預報
- 氣象預警資訊

### 📊 系統監控（新增功能）
- **CPU 使用率監控** - 即時顯示處理器使用率
- **GPU 使用率監控** - 顯示顯卡負載狀況  
- **RAM 記憶體監控** - 顯示記憶體使用情況
- **藍牙無線傳輸** - Windows 電腦透過藍牙傳送資料
- **輕量化程式** - Windows 後台程式，支援開機自啟動

## 目錄
- `platformio.ini`：建置設定
- `src/ESP32InfoBoard_TFT.ino`：主程式
- `include/icons_tft.h`：簡易天氣圖示繪製
- `windows_monitor/`：Windows 系統監控程式
  - `system_monitor.py`：主監控程式
  - `install.bat`：自動安裝腳本
  - `README.md`：詳細使用說明
- `preview/index.html` + `preview/widget.css`：前端排版預覽

## 硬體需求
- ESP32 開發板（ESP32-DevKitC 或同等）
- 2.8" ILI9341 TFT 320×240（SPI）
- 預設接腳：SCK=18, MISO=19, MOSI=23, CS=5, DC=2, RST=4（可於程式開頭修改）

## 編譯與燒錄（PlatformIO）
1. 安裝 VS Code + PlatformIO IDE
2. 打開此資料夾
3. 接上板子，點擊「Upload」即可
4. 串列監控 115200 bps

必要函式庫（`lib_deps` 已自動安裝）：
- Adafruit GFX Library
- Adafruit ILI9341
- ArduinoJson

## 首次使用
1. 開機若 WiFi 無法連上，會轉為 AP 模式：
   - SSID：`ESP32-InfoBoard-Setup`
   - 密碼：`12345678`
   - 打開 `http://192.168.4.1` 進行設定
2. 配置頁填入：
   - WiFi SSID/密碼
   - QWeather API Key（https://dev.qweather.com）
   - 城市名（系統會自動查詢地區 ID）
3. 保存後裝置會自動重啟並抓取天氣

## Windows 系統監控設定

### 1. ESP32 藍牙配對
1. 確保 ESP32 已燒錄最新固件並開機
2. 在 Windows 藍牙設定中搜尋「ESP32-InfoBoard」
3. 配對並記下 COM 埠號

### 2. 安裝 Windows 監控程式
1. 進入 `windows_monitor` 資料夾
2. 雙擊執行 `install.bat` 進行自動安裝
3. 安裝完成後執行 `start_monitor.bat` 啟動監控
4. 可選：執行 `autostart_setup.bat` 設定開機自啟動

### 3. 監控功能
- **CPU 使用率**：即時處理器負載百分比
- **GPU 使用率**：顯卡使用率（需支援的顯卡）
- **RAM 使用率**：記憶體使用情況（已用/總量）
- **連線狀態**：綠點表示已連線，紅點表示斷線
- **更新頻率**：每 2 秒更新一次

## API 與資料
- 現在天氣：`/api/update-now`
- 三日預報：`/api/update-forecast`
- 氣象預警：`/api/update-warning`
- 合併查詢（只讀）：`/api/weather`

已完全移除 UV 生活指數，程式碼與 JSON 皆不含該欄位。

## 版面與顯示
- 畫面切區：
  - Header 0–24px（地區與 IP）
  - 第一行：時間（左）與現在天氣（右）26–140px
  - 第二行：系統監控（CPU/GPU/RAM）142–232px
  - 底部：預警 235–257px
- 「現在天氣」卡片：
  - 上方：大圖示 + 溫度
  - 兩行文字：`現象` 與 `濕度 · 風`
  - 內建等寬字體自動縮寫與左右分攤策略，確保永不截斷
- 「系統監控」卡片：
  - 三個監控卡片：CPU、GPU、RAM
  - 顯示使用率百分比、進度條、額外資訊
  - 連線狀態指示燈（綠色=已連線，紅色=斷線）

## 檢查清單（已逐項確認）
- [x] 移除 UV：無 `uv` / `life` 相關程式碼或 UI
- [x] 兩行資訊完整顯示：在 108px 欄寬內不會出現 `...`
- [x] Grid/座標不重疊：各段高度/間距與座標互相對齊
- [x] Web 配置可存取：AP 模式啟動 + `/api/save` 後自動重啟
- [x] TLS：使用 `WiFiClientSecure` + `setInsecure()`（簡化；如需驗證可換 Root CA）
- [x] 字型：使用 Adafruit GFX 內建與 FreeSans 字體（隨庫提供）
- [x] 只需 3 個 API：`now/3d/warning`，不使用其他端點
- [x] 藍牙系統監控：支援 Windows 電腦透過藍牙傳送 CPU/GPU/RAM 資料
- [x] 輕量化設計：Windows 背景程式支援開機自啟動

## 常見問題

### 天氣相關
- 城市找不到 ID？
  - 確認有填 API Key，城市可用中/英/拼音；後端只取第一筆結果
- HTTPS 失敗？
  - 若目標網路限制 SNI/證書，請考慮改用固定 Root CA 或 HTTP 代理
- 顯示被蓋住？
  - TFT 旋轉請維持 `tft.setRotation(1)`；更改後需調整區塊座標

### 系統監控相關
- 藍牙無法連線？
  - 確認 Windows 已配對 ESP32 裝置
  - 檢查 COM 埠是否正確
  - 重新啟動 Windows 監控程式
- GPU 資訊顯示 "--"？
  - 確認已安裝 GPUtil：`pip install GPUtil`
  - 檢查顯卡驅動是否最新
- 系統監控卡片顯示「未連線」？
  - 超過 10 秒未收到資料會顯示未連線
  - 檢查 Windows 程式是否正常運行

## 預覽（HTML/CSS）
打開 `preview/index.html`，即可看到 iOS widget 風格排版。此為純視覺用，不與裝置互動。

## 授權
- 你可自由使用與修改本專案於個人或專題用途。