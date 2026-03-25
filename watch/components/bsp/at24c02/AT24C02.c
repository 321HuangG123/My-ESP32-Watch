#include "AT24C02.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include <string.h>
#include "i2c/my_i2c.h"



// 定义 EEPROM 使用的 I2C 总线对象
iic_bus_t AT_bus = {
    .i2c_port = I2C_NUM_0, // 与 MPU6050 共用总线或使用独立总线
    .timeout_ms = 100
};

void AT24C02_Write(uint8_t addr,uint8_t length,uint8_t buff[])
{
	IIC_Write_Multi_Byte(&AT_bus, AT_ADDRESS, addr, length, buff);

	// 关键：EEPROM 写入后必须延时至少 5ms 才能进行下一次操作
    vTaskDelay(pdMS_TO_TICKS(10));
}


void AT24C02_Read(uint8_t addr, uint8_t length, uint8_t buff[])
{
	IIC_Read_Multi_Byte(&AT_bus, AT_ADDRESS, addr, length, buff);
}


void AT24C02_Init(void)
{
	IICInit(&AT_bus, 11, 12, 400000);
}
