#!/usr/bin/env python3
"""
Windows System Monitor for ESP32 InfoBoard
輕量化系統監控程式，透過藍牙傳送 CPU、GPU、RAM 使用率到 ESP32

需求套件：
pip install psutil bluetooth-serial GPUtil
"""

import json
import time
import threading
import psutil
import serial.tools.list_ports
import serial
from datetime import datetime
import logging
import sys
import os

# 設定日誌
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('system_monitor.log'),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

class SystemMonitor:
    def __init__(self):
        self.esp32_device = None
        self.serial_connection = None
        self.running = False
        self.esp32_name = "ESP32-InfoBoard"  # 藍牙裝置名稱
        self.reconnect_interval = 5  # 重連間隔（秒）
        
    def find_esp32_bluetooth(self):
        """尋找 ESP32 藍牙裝置"""
        try:
            ports = serial.tools.list_ports.comports()
            for port in ports:
                if "ESP32" in port.description or "ESP32" in port.device:
                    logger.info(f"發現可能的 ESP32 裝置: {port.device} - {port.description}")
                    return port.device
            
            # 如果沒找到，嘗試常見的藍牙 COM 埠
            for i in range(1, 20):
                port_name = f"COM{i}"
                try:
                    test_serial = serial.Serial(port_name, 115200, timeout=1)
                    test_serial.close()
                    logger.info(f"嘗試使用 COM 埠: {port_name}")
                    return port_name
                except:
                    continue
            
            return None
        except Exception as e:
            logger.error(f"尋找藍牙裝置時發生錯誤: {e}")
            return None
    
    def connect_to_esp32(self):
        """連接到 ESP32"""
        try:
            if self.serial_connection and self.serial_connection.is_open:
                return True
                
            port = self.find_esp32_bluetooth()
            if not port:
                logger.warning("未找到 ESP32 裝置")
                return False
            
            self.serial_connection = serial.Serial(
                port=port,
                baudrate=115200,
                timeout=2,
                write_timeout=2
            )
            
            if self.serial_connection.is_open:
                logger.info(f"已連接到 ESP32: {port}")
                return True
            else:
                logger.error("無法開啟串列埠連接")
                return False
                
        except Exception as e:
            logger.error(f"連接 ESP32 時發生錯誤: {e}")
            return False
    
    def get_gpu_usage(self):
        """獲取 GPU 使用率（需要 GPUtil）"""
        try:
            import GPUtil
            gpus = GPUtil.getGPUs()
            if gpus:
                # 返回第一個 GPU 的使用率
                return gpus[0].load * 100, f"{gpus[0].temperature}°C"
            else:
                return 0.0, "--"
        except ImportError:
            logger.warning("GPUtil 未安裝，無法獲取 GPU 資訊")
            return 0.0, "--"
        except Exception as e:
            logger.error(f"獲取 GPU 資訊時發生錯誤: {e}")
            return 0.0, "--"
    
    def get_cpu_temperature(self):
        """獲取 CPU 溫度（Windows 平台較複雜，暫時返回虛擬值）"""
        try:
            # Windows 平台獲取溫度較複雜，這裡返回模擬值
            # 實際應用中可以使用 WMI 或其他工具
            return "--"
        except:
            return "--"
    
    def collect_system_data(self):
        """收集系統資料"""
        try:
            # CPU 使用率
            cpu_usage = psutil.cpu_percent(interval=1)
            
            # GPU 使用率和溫度
            gpu_usage, gpu_temp = self.get_gpu_usage()
            
            # 記憶體使用率
            memory = psutil.virtual_memory()
            ram_usage = memory.percent
            ram_total = round(memory.total / (1024**3), 1)  # GB
            ram_used = round(memory.used / (1024**3), 1)   # GB
            
            # CPU 溫度
            cpu_temp = self.get_cpu_temperature()
            
            return {
                "type": "system",
                "timestamp": datetime.now().isoformat(),
                "cpu_usage": round(cpu_usage, 1),
                "gpu_usage": round(gpu_usage, 1),
                "ram_usage": round(ram_usage, 1),
                "ram_total": ram_total,
                "ram_used": ram_used,
                "cpu_temp": cpu_temp,
                "gpu_temp": gpu_temp
            }
        except Exception as e:
            logger.error(f"收集系統資料時發生錯誤: {e}")
            return None
    
    def send_data(self, data):
        """發送資料到 ESP32"""
        try:
            if not self.serial_connection or not self.serial_connection.is_open:
                return False
            
            json_data = json.dumps(data, ensure_ascii=False)
            message = json_data + '\n'
            
            self.serial_connection.write(message.encode('utf-8'))
            self.serial_connection.flush()
            
            logger.debug(f"已發送資料: {json_data}")
            return True
            
        except Exception as e:
            logger.error(f"發送資料時發生錯誤: {e}")
            self.serial_connection = None
            return False
    
    def monitor_loop(self):
        """主監控迴圈"""
        logger.info("系統監控已啟動")
        
        while self.running:
            try:
                # 檢查連線狀態
                if not self.connect_to_esp32():
                    logger.warning(f"等待 {self.reconnect_interval} 秒後重試連線...")
                    time.sleep(self.reconnect_interval)
                    continue
                
                # 收集系統資料
                system_data = self.collect_system_data()
                if system_data:
                    # 發送資料
                    if self.send_data(system_data):
                        logger.info(f"CPU: {system_data['cpu_usage']}%, "
                                  f"GPU: {system_data['gpu_usage']}%, "
                                  f"RAM: {system_data['ram_usage']}%")
                    else:
                        logger.warning("發送資料失敗，將重新連線")
                        self.serial_connection = None
                
                # 等待 2 秒
                time.sleep(2)
                
            except KeyboardInterrupt:
                logger.info("收到中斷信號，正在停止...")
                break
            except Exception as e:
                logger.error(f"監控迴圈發生錯誤: {e}")
                time.sleep(self.reconnect_interval)
    
    def start(self):
        """啟動監控"""
        self.running = True
        self.monitor_loop()
    
    def stop(self):
        """停止監控"""
        self.running = False
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        logger.info("系統監控已停止")

def main():
    """主程式"""
    logger.info("ESP32 系統監控程式啟動")
    logger.info("按 Ctrl+C 停止程式")
    
    monitor = SystemMonitor()
    
    try:
        monitor.start()
    except KeyboardInterrupt:
        logger.info("收到停止信號")
    finally:
        monitor.stop()

if __name__ == "__main__":
    main()