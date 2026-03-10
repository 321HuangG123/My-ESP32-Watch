idf_component_register( SRCS    "bmp280/bmp280.c"               # 气压                  
                                "dht11/dht11.c"                 # 温湿度
                                "key/key.c"                     # 按键
                                "lsm303/lsm303.c"               # 电子罗盘
                                "max30102/max30102.c" "max30102/max30102_algorithm.c" "max30102/max30102_read.c"    # 心率血氧
                                "mpu6050/inv_mpu.c" "mpu6050/inv_mpu_dmp_motion_driver.c" "mpu6050/mpu6050.c"       # 三轴加速度角度
                                "screen/cst816t_driver.c" "screen/st7789_driver.c"      # 屏幕

                        INCLUDE_DIRS    "."

                        REQUIRES    "esp_lcd" 
                                    "driver"
                                    "nvs_flash" 
)