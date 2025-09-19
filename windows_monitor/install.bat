@echo off
echo ESP32 系統監控程式安裝腳本
echo =============================

echo 1. 檢查 Python 環境...
python --version >nul 2>&1
if errorlevel 1 (
    echo 錯誤: 未找到 Python！請先安裝 Python 3.7+
    echo 下載地址: https://www.python.org/downloads/
    pause
    exit /b 1
)

echo Python 已安裝

echo 2. 安裝必要套件...
pip install -r requirements.txt

if errorlevel 1 (
    echo 錯誤: 套件安裝失敗
    pause
    exit /b 1
)

echo 3. 創建啟動腳本...
echo @echo off > start_monitor.bat
echo echo ESP32 系統監控程式 >> start_monitor.bat
echo echo ================== >> start_monitor.bat
echo python system_monitor.py >> start_monitor.bat
echo pause >> start_monitor.bat

echo 4. 創建開機自啟動腳本...
set "current_dir=%cd%"
echo @echo off > autostart_setup.bat
echo echo 設定開機自啟動 >> autostart_setup.bat
echo echo ============== >> autostart_setup.bat
echo copy "%current_dir%\start_monitor.bat" "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\Startup\" >> autostart_setup.bat
echo echo 已設定開機自啟動 >> autostart_setup.bat
echo pause >> autostart_setup.bat

echo.
echo 安裝完成！
echo ==========
echo 使用方式：
echo 1. 雙擊 start_monitor.bat 啟動監控程式
echo 2. 雙擊 autostart_setup.bat 設定開機自啟動
echo 3. 確保 ESP32 已配對藍牙並開機
echo.
pause