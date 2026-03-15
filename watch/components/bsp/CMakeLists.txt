idf_component_register( SRCS    "dht11/dht11.c"                 # 温湿度
                                "key/key.c"                     # 按键
                                "lsm303/lsm303.c"               # 电子罗盘
                                "max30102/max30102.c" "max30102/max30102_algorithm.c" "max30102/max30102_read.c"    # 心率血压 
                                "spl06_001/spl06_001.c"         # 气压         
                                "mpu6050/inv_mpu.c" "mpu6050/inv_mpu_dmp_motion_driver.c" "mpu6050/mpu6050.c"       # 三轴加速度角度
                                "screen/CST816.c"   "screen/lcd_init.c"   "screen/lcd.c"      # 屏幕
                                "at24c02/AT24C02.c" "at24c02/DataSave.c"                # E2PROM数据存储

                        INCLUDE_DIRS    "."

                        REQUIRES    "esp_lcd" 
                                    "driver"
                                    "nvs_flash" 
)