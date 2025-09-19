#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Windows System Monitor for ESP32 InfoBoard
輕量化系統監控程式，透過藍牙傳送 CPU、GPU、RAM 使用率到 ESP32

需求套件：
pip install psutil pyserial GPUtil

Windows 11 相容性：
- 支援 UTF-8 編碼
- 改進串口檢測
- 增強錯誤處理
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
import platform

# 確保 UTF-8 編碼（Windows 11 兼容性）
if sys.platform == "win32":
    import codecs
    sys.stdout = codecs.getwriter("utf-8")(sys.stdout.detach())
    sys.stderr = codecs.getwriter("utf-8")(sys.stderr.detach())

# 設定日誌
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('system_monitor.log', encoding='utf-8'),
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
            logger.info("正在搜尋 ESP32 藍牙裝置...")
            ports = serial.tools.list_ports.comports()
            
            # 記錄系統資訊用於診斷
            logger.info(f"作業系統: {platform.system()} {platform.release()}")
            logger.info(f"發現 {len(ports)} 個串列埠")
            
            # 優先搜尋包含 ESP32 關鍵字的裝置
            for port in ports:
                port_info = f"{port.device} - {port.description}"
                logger.info(f"檢查串列埠: {port_info}")
                
                # Windows 11 可能有不同的裝置描述
                esp32_keywords = ["ESP32", "esp32", "Standard Serial over Bluetooth link", 
                                "Bluetooth", "bluetooth", "Serial", "CP210x", "CH340"]
                
                for keyword in esp32_keywords:
                    if keyword in port.description or keyword in port.device:
                        logger.info(f"發現可能的 ESP32 裝置: {port.device} - {port.description}")
                        
                        # 測試連線可用性
                        if self.test_port_connection(port.device):
                            return port.device
            
            # 如果沒找到明確的 ESP32 裝置，嘗試常見的藍牙 COM 埠
            logger.info("未找到明確的 ESP32 裝置，嘗試常見的 COM 埠...")
            for i in range(1, 21):  # 增加搜尋範圍到 COM20
                port_name = f"COM{i}"
                if self.test_port_connection(port_name):
                    logger.info(f"成功測試 COM 埠: {port_name}")
                    return port_name
            
            logger.warning("未找到可用的 ESP32 藍牙連接")
            logger.info("請確認:")
            logger.info("1. ESP32 已開機並啟動藍牙")
            logger.info("2. Windows 藍牙設定中已配對 'ESP32-InfoBoard'")
            logger.info("3. ESP32 藍牙裝置顯示為已連接狀態")
            
            return None
            
        except Exception as e:
            logger.error(f"尋找藍牙裝置時發生錯誤: {e}")
            return None
    
    def test_port_connection(self, port_name):
        """測試串列埠連接可用性"""
        try:
            test_serial = serial.Serial(
                port=port_name, 
                baudrate=115200, 
                timeout=0.5,
                write_timeout=0.5
            )
            test_serial.close()
            return True
        except (serial.SerialException, OSError, ValueError) as e:
            # 正常的無法連接情況，不記錄為錯誤
            return False
        except Exception as e:
            logger.debug(f"測試 {port_name} 時發生錯誤: {e}")
            return False
    
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
    logger.info(f"Python 版本: {sys.version}")
    logger.info(f"作業系統: {platform.system()} {platform.release()}")
    logger.info("按 Ctrl+C 停止程式")
    
    # Windows 11 兼容性檢查
    if platform.system() == "Windows":
        major_version = int(platform.release())
        if major_version >= 10:
            logger.info("檢測到 Windows 10/11，啟用兼容性模式")
            
    # 檢查必要套件
    try:
        import psutil
        logger.info(f"psutil 版本: {psutil.__version__}")
    except ImportError:
        logger.error("psutil 未安裝，請執行: pip install psutil")
        input("按 Enter 鍵關閉...")
        return
        
    try:
        import serial
        logger.info(f"pyserial 版本: {serial.__version__}")
    except ImportError:
        logger.error("pyserial 未安裝，請執行: pip install pyserial")
        input("按 Enter 鍵關閉...")
        return
    
    try:
        import GPUtil
        logger.info("GPUtil 已載入 (GPU 監控可用)")
    except ImportError:
        logger.warning("GPUtil 未安裝，GPU 監控將顯示 0% (可選安裝: pip install GPUtil)")
    
    monitor = SystemMonitor()
    
    try:
        monitor.start()
    except KeyboardInterrupt:
        logger.info("收到停止信號")
    except Exception as e:
        logger.error(f"程式執行時發生錯誤: {e}")
        logger.error("請檢查:")
        logger.error("1. Python 環境是否正確安裝")
        logger.error("2. 必要套件是否已安裝")
        logger.error("3. ESP32 是否已正確配對藍牙")
        input("按 Enter 鍵關閉...")
    finally:
        monitor.stop()

if __name__ == "__main__":
    main()