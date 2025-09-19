@echo off
chcp 65001 >nul 2>&1
echo ESP32 系統監控診斷工具
echo ========================
echo.

echo 1. 檢查作業系統...
echo 作業系統: %OS%
ver

echo.
echo 2. 檢查 Python 環境...
python --version 2>&1
if errorlevel 1 (
    echo Python 命令不可用，嘗試 py 命令...
    py --version 2>&1
    if errorlevel 1 (
        echo ❌ 未找到 Python 安裝
        echo 請安裝 Python 3.7 或更新版本
    ) else (
        echo ✅ 發現 Python Launcher
        set PYTHON_CMD=py
    )
) else (
    echo ✅ 發現 Python
    set PYTHON_CMD=python
)

echo.
echo 3. 檢查 Python 套件...
if defined PYTHON_CMD (
    echo 檢查 psutil...
    %PYTHON_CMD% -c "import psutil; print('✅ psutil 版本:', psutil.__version__)" 2>nul || echo ❌ psutil 未安裝
    
    echo 檢查 pyserial...
    %PYTHON_CMD% -c "import serial; print('✅ pyserial 版本:', serial.__version__)" 2>nul || echo ❌ pyserial 未安裝
    
    echo 檢查 GPUtil...
    %PYTHON_CMD% -c "import GPUtil; print('✅ GPUtil 可用')" 2>nul || echo ⚠️ GPUtil 未安裝 (可選)
)

echo.
echo 4. 檢查串列埠...
if defined PYTHON_CMD (
    %PYTHON_CMD% -c "import serial.tools.list_ports; ports = list(serial.tools.list_ports.comports()); print(f'發現 {len(ports)} 個串列埠:'); [print(f'  {p.device}: {p.description}') for p in ports]" 2>nul || echo ❌ 無法檢查串列埠
)

echo.
echo 5. 檢查藍牙服務...
sc query bthserv | find "RUNNING" >nul && echo ✅ 藍牙服務正在執行 || echo ❌ 藍牙服務未運行

echo.
echo 6. 檢查防火牆和防毒軟體...
echo 請確認防毒軟體未阻擋 Python 或串列埠存取

echo.
echo 7. 系統建議...
echo - 確保 ESP32 已配對並顯示為 "已連接"
echo - 嘗試重新啟動藍牙服務
echo - 檢查 Windows 更新
echo - 確認 ESP32 韌體版本為最新

echo.
echo 診斷完成！
echo 如果問題持續，請將此輸出截圖並聯繫技術支援。
echo.
pause