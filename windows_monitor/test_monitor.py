#!/usr/bin/env python3
"""
ESP32 Windows Monitor - Verification Test Script
用於驗證系統監控程式的核心功能

執行方式: python test_monitor.py
"""

import json
import sys
import os

def test_imports():
    """測試必要套件是否可正常導入"""
    try:
        import psutil
        print("✅ psutil 導入成功")
    except ImportError:
        print("❌ psutil 未安裝，請執行: pip install psutil")
        return False
    
    try:
        import serial
        print("✅ pyserial 導入成功")
    except ImportError:
        print("❌ pyserial 未安裝，請執行: pip install pyserial")
        return False
    
    try:
        import GPUtil
        print("✅ GPUtil 導入成功")
    except ImportError:
        print("⚠️  GPUtil 未安裝，GPU 監控功能將無法使用")
        print("   可選安裝: pip install GPUtil")
    
    return True

def test_system_info():
    """測試系統資訊讀取功能"""
    try:
        import psutil
        
        # 測試 CPU 使用率
        cpu_usage = psutil.cpu_percent(interval=1)
        print(f"✅ CPU 使用率: {cpu_usage}%")
        
        # 測試記憶體資訊
        memory = psutil.virtual_memory()
        ram_usage = memory.percent
        ram_total = memory.total / (1024**3)  # GB
        ram_used = memory.used / (1024**3)   # GB
        print(f"✅ RAM 使用率: {ram_usage}% ({ram_used:.1f}/{ram_total:.1f} GB)")
        
        # 測試 GPU 資訊
        try:
            import GPUtil
            gpus = GPUtil.getGPUs()
            if gpus:
                gpu = gpus[0]
                print(f"✅ GPU 使用率: {gpu.load*100:.1f}% (溫度: {gpu.temperature}°C)")
            else:
                print("⚠️  未檢測到支援的 GPU")
        except:
            print("⚠️  GPU 資訊讀取失敗")
        
        return True
    except Exception as e:
        print(f"❌ 系統資訊讀取失敗: {e}")
        return False

def test_json_format():
    """測試 JSON 資料格式生成"""
    try:
        import psutil
        from datetime import datetime
        
        # 模擬系統資料
        data = {
            "type": "system",
            "timestamp": datetime.now().isoformat(),
            "cpu_usage": psutil.cpu_percent(),
            "gpu_usage": 0.0,  # 預設值
            "ram_usage": psutil.virtual_memory().percent,
            "ram_total": psutil.virtual_memory().total / (1024**3),
            "ram_used": psutil.virtual_memory().used / (1024**3),
            "cpu_temp": "--",
            "gpu_temp": "--"
        }
        
        # 生成 JSON
        json_str = json.dumps(data, indent=2)
        print("✅ JSON 格式生成成功:")
        print(json_str)
        
        # 驗證 JSON 解析
        parsed = json.loads(json_str)
        print("✅ JSON 解析驗證成功")
        
        return True
    except Exception as e:
        print(f"❌ JSON 格式測試失敗: {e}")
        return False

def test_serial_ports():
    """測試串列埠列舉功能"""
    try:
        import serial.tools.list_ports
        
        ports = serial.tools.list_ports.comports()
        print(f"✅ 找到 {len(ports)} 個串列埠:")
        
        for port in ports:
            print(f"   - {port.device}: {port.description}")
            
        if not ports:
            print("⚠️  未找到任何串列埠，請確認:")
            print("   1. ESP32 已連接並開機")
            print("   2. Windows 已完成藍牙配對")
            print("   3. ESP32 藍牙功能正常")
        
        return True
    except Exception as e:
        print(f"❌ 串列埠測試失敗: {e}")
        return False

def main():
    """主測試流程"""
    print("ESP32 Windows Monitor - 功能驗證測試")
    print("=" * 50)
    
    tests = [
        ("套件導入測試", test_imports),
        ("系統資訊測試", test_system_info),
        ("JSON 格式測試", test_json_format),
        ("串列埠測試", test_serial_ports)
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n🧪 {test_name}")
        print("-" * 30)
        if test_func():
            passed += 1
            print(f"✅ {test_name} 通過")
        else:
            print(f"❌ {test_name} 失敗")
    
    print("\n" + "=" * 50)
    print(f"測試結果: {passed}/{total} 項測試通過")
    
    if passed == total:
        print("🎉 所有測試通過！系統監控程式已就緒。")
        print("\n下一步:")
        print("1. 確認 ESP32 已配對藍牙")
        print("2. 執行 python system_monitor.py 啟動監控")
    else:
        print("⚠️  部分測試失敗，請檢查環境配置。")
        
    return passed == total

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)