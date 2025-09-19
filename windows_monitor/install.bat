@echo off
chcp 65001 >nul 2>&1
echo ESP32 系統監控程式安裝腳本
echo =============================

echo 1. 檢查 Python 環境...
python --version >nul 2>&1
if errorlevel 1 (
    py --version >nul 2>&1
    if errorlevel 1 (
        echo 錯誤: 未找到 Python！請先安裝 Python 3.7+
        echo 下載地址: https://www.python.org/downloads/
        echo.
        echo 注意: Windows 11 用戶請確保從 Microsoft Store 或官網安裝 Python
        pause
        exit /b 1
    ) else (
        echo 發現 Python Launcher, 將使用 py 命令
        set PYTHON_CMD=py
    )
) else (
    set PYTHON_CMD=python
)

echo %PYTHON_CMD% 已安裝

echo 2. 安裝必要套件...
%PYTHON_CMD% -m pip install -r requirements.txt

if errorlevel 1 (
    echo 錯誤: 套件安裝失敗
    echo 嘗試使用 --user 安裝...
    %PYTHON_CMD% -m pip install --user -r requirements.txt
    if errorlevel 1 (
        echo 錯誤: 套件安裝仍然失敗，請檢查網路連線或權限
        pause
        exit /b 1
    )
)

echo 3. 創建啟動腳本...
echo @echo off > start_monitor.bat
echo chcp 65001 ^>nul 2^>^&1 >> start_monitor.bat
echo echo ESP32 系統監控程式 >> start_monitor.bat
echo echo ================== >> start_monitor.bat
echo echo 正在檢查 Python 環境... >> start_monitor.bat
echo %PYTHON_CMD% --version ^>nul 2^>^&1 >> start_monitor.bat
echo if errorlevel 1 ^( >> start_monitor.bat
echo     py --version ^>nul 2^>^&1 >> start_monitor.bat
echo     if errorlevel 1 ^( >> start_monitor.bat
echo         echo 錯誤: 未找到 Python！ >> start_monitor.bat
echo         pause >> start_monitor.bat
echo         exit /b 1 >> start_monitor.bat
echo     ^) else ^( >> start_monitor.bat
echo         py system_monitor.py >> start_monitor.bat
echo     ^) >> start_monitor.bat
echo ^) else ^( >> start_monitor.bat
echo     %PYTHON_CMD% system_monitor.py >> start_monitor.bat
echo ^) >> start_monitor.bat
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