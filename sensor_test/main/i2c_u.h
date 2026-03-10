#ifndef __I2C_U_H__
#define __I2C_U_H__

#include "driver/i2c.h"

// 定义适配 ESP32 的总线结构体
typedef struct {
    i2c_port_t port;      // I2C 端口号 (I2C_NUM_0 或 I2C_NUM_1)
    uint32_t scl_io;      // SCL 引脚编号
    uint32_t sda_io;      // SDA 引脚编号
} esp32_i2c_bus_t;

// 声明全局变量供 SPL06 使用
extern esp32_i2c_bus_t SPL_bus;


// 写入一个字节
uint8_t IIC_Write_One_Byte(esp32_i2c_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t data);

// 读取一个字节
unsigned char IIC_Read_One_Byte(esp32_i2c_bus_t *bus, uint8_t daddr, uint8_t reg);

// 初始化硬件 I2C
void IICInit(esp32_i2c_bus_t *bus);

#endif
