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
import socket

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
        self.network_socket = None  # 網路套接字用於觸發防火牆警告
        
    def init_network_socket(self):
        """初始化網路套接字以觸發 Windows 防火牆警告"""
        try:
            # 創建 UDP 套接字（常用於設備發現和通訊）
            self.network_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            # 綁定到本地端口（這會觸發 Windows 防火牆警告）
            self.network_socket.bind(('0.0.0.0', 0))  # 使用任意可用端口
            local_port = self.network_socket.getsockname()[1]
            logger.info(f"網路套接字已初始化，監聽端口: {local_port}")
            logger.info("Windows 防火牆可能會詢問是否允許此程式存取網路")
            return True
        except Exception as e:
            logger.warning(f"無法初始化網路套接字: {e}")
            return False
    
    def check_network_discovery(self):
        """檢查網路設備發現功能（觸發更多網路活動）"""
        try:
            if not self.network_socket:
                return False
                
            # 嘗試發送 UDP 廣播包進行設備發現（這是合法的網路操作）
            broadcast_msg = json.dumps({
                "type": "discovery", 
                "app": "ESP32-Monitor",
                "timestamp": datetime.now().isoformat()
            }).encode('utf-8')
            
            # 發送到本地廣播地址
            try:
                self.network_socket.sendto(broadcast_msg, ('127.0.0.1', 12345))
                logger.debug("網路發現包已發送")
            except:
                pass  # 忽略發送錯誤，這只是為了觸發防火牆
                
            return True
        except Exception as e:
            logger.debug(f"網路發現檢查失敗: {e}")
            return False
    
    def test_network_connectivity(self):
        """測試網路連線能力（確保防火牆檢測）"""
        try:
            # 嘗試創建一個 TCP 連接測試（會觸發防火牆警告）
            test_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            test_socket.settimeout(1)
            
            # 嘗試連接到本地回環地址（安全的測試）
            try:
                test_socket.connect(('127.0.0.1', 80))
            except:
                pass  # 連接失敗是正常的，我們只是要觸發防火牆
            finally:
                test_socket.close()
                
            logger.debug("網路連線能力測試完成")
            return True
        except Exception as e:
            logger.debug(f"網路連線測試失敗: {e}")
            return False
    
    def close_network_socket(self):
        """關閉網路套接字"""
        if self.network_socket:
            try:
                self.network_socket.close()
                logger.info("網路套接字已關閉")
            except:
                pass
            self.network_socket = None
        
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
                    # 執行網路發現檢查（確保防火牆感知）
                    self.check_network_discovery()
                    
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
        # 首先初始化網路套接字以觸發防火牆警告
        logger.info("正在初始化網路功能...")
        self.init_network_socket()
        
        # 執行網路連線測試（進一步確保防火牆警告）
        self.test_network_connectivity()
        
        self.running = True
        self.monitor_loop()
    
    def stop(self):
        """停止監控"""
        self.running = False
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        self.close_network_socket()
        logger.info("系統監控已停止")

def main():
    """主程式"""
    logger.info("ESP32 系統監控程式啟動")
    logger.info("==========================")
    logger.info("注意：此程式會使用網路功能進行設備通訊")
    logger.info("Windows 防火牆可能會詢問是否允許網路存取，請選擇「允許」")
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