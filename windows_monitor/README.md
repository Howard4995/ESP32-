# ESP32 系統監控 - Windows 應用程式

這是一個輕量化的 Windows 系統監控程式，會透過藍牙將 CPU、GPU、RAM 使用率傳送到 ESP32 信息牌顯示。

## 功能特色

- 📊 即時監控 CPU、GPU、RAM 使用率
- 🔵 透過藍牙無線傳輸到 ESP32
- 🚀 輕量化設計，佔用資源極少
- 🔄 開機自啟動支援
- 📱 在 ESP32 TFT 螢幕上可視化顯示

## 安裝與使用

### 1. 系統需求

- Windows 10/11
- Python 3.7 或更新版本
- ESP32 裝置（已燒錄最新固件）
- 藍牙功能

### 2. 安裝步驟

1. **下載並解壓縮**程式到任意資料夾
2. **雙擊執行 `install.bat`** 進行自動安裝
3. 安裝完成後會產生以下文件：
   - `start_monitor.bat` - 啟動監控程式
   - `autostart_setup.bat` - 設定開機自啟動

### 3. 配對 ESP32 藍牙

1. 確保 ESP32 已開機並啟動藍牙（裝置名稱：ESP32-InfoBoard）
2. 在 Windows 藍牙設定中搜尋並配對 ESP32
3. 記下配對後的 COM 埠號（例如：COM3）

### 4. 啟動監控

**手動啟動：**
```batch
雙擊 start_monitor.bat
```

**開機自啟動：**
```batch
雙擊 autostart_setup.bat（管理員權限）
```

**診斷工具：**
```batch
雙擊 diagnose.bat（系統診斷）
```

## 資料格式

程式會定期（每2秒）發送以下 JSON 格式資料到 ESP32：

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

## 故障排除

### 連線問題
- 確認 ESP32 藍牙已啟動
- 檢查 Windows 藍牙設定中是否已配對
- 重新配對藍牙裝置

### 程式無法啟動
- 確認已安裝 Python 3.7+
- Windows 11 用戶建議從 Microsoft Store 安裝 Python
- 重新執行 `install.bat`
- 檢查防毒軟體是否阻擋
- 嘗試以管理員權限執行

### Windows 11 特殊設定
- 執行 `diagnose.bat` 進行系統診斷
- 確認 Windows 編碼設定為 UTF-8
- 檢查藍牙服務是否正常運行
- 嘗試重新安裝藍牙驅動程式

### GPU 資訊無法顯示
- 確認已安裝 NVIDIA 或 AMD 顯卡驅動
- GPUtil 套件僅支援部分顯卡

## 進階設定

可以修改 `system_monitor.py` 中的以下參數：

```python
self.reconnect_interval = 5  # 重連間隔（秒）
time.sleep(2)               # 資料發送間隔（秒）
```

## 日誌檔案

程式執行時會產生 `system_monitor.log` 記錄運行狀態，可用於問題診斷。