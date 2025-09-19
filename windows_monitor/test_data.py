#!/usr/bin/env python3
"""
ESP32 系統監控示例程式
模擬發送系統資料到 ESP32（用於測試）
"""

import json
import time
import random

def generate_test_data():
    """產生測試用的系統監控資料"""
    return {
        "type": "system",
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
        "cpu_usage": round(random.uniform(20, 80), 1),
        "gpu_usage": round(random.uniform(10, 90), 1),
        "ram_usage": round(random.uniform(30, 70), 1),
        "ram_total": 16.0,
        "ram_used": round(random.uniform(4, 12), 1),
        "cpu_temp": f"{random.randint(45, 75)}°C",
        "gpu_temp": f"{random.randint(50, 80)}°C"
    }

def main():
    print("ESP32 系統監控測試程式")
    print("======================")
    print("模擬資料格式（每2秒輸出一次）：")
    print()
    
    try:
        while True:
            data = generate_test_data()
            json_str = json.dumps(data, ensure_ascii=False, indent=2)
            
            print(f"時間: {time.strftime('%H:%M:%S')}")
            print(json_str)
            print("-" * 50)
            
            time.sleep(2)
            
    except KeyboardInterrupt:
        print("\n程式已停止")

if __name__ == "__main__":
    main()