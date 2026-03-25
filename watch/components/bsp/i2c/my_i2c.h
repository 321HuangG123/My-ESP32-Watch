#ifndef __MY_I2C_H__
#define __MY_I2C_H__

#include "driver/i2c.h"


// 重新定义总线结构体，仅保留端口号和超时定义
typedef struct {
    i2c_port_t i2c_port;     // I2C_NUM_0 或 I2C_NUM_1
    uint32_t timeout_ms;     // 超时时间
} iic_bus_t;

void IICInit(iic_bus_t *bus, int sda_io, int scl_io, uint32_t freq_hz);
uint8_t IIC_Write_One_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t data);
uint8_t IIC_Write_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]);
unsigned char IIC_Read_One_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg);
uint8_t IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]);

#endif
