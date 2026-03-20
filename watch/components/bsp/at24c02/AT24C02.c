#include "AT24C02.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include <string.h>

// 重新定义总线结构体，仅保留端口号和超时定义
typedef struct {
    i2c_port_t i2c_port;     // I2C_NUM_0 或 I2C_NUM_1
    uint32_t timeout_ms;     // 超时时间
} iic_bus_t;

/**
 * @brief 初始化硬件 I2C
 */
void IICInit(iic_bus_t *bus, int sda_io, int scl_io, uint32_t freq_hz) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_io,
        .scl_io_num = scl_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq_hz,
    };
    i2c_param_config(bus->i2c_port, &conf);
    i2c_driver_install(bus->i2c_port, conf.mode, 0, 0, 0);
}

/**
 * @brief I2C 写单字节
 */
uint8_t IIC_Write_One_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    esp_err_t err = i2c_master_write_to_device(bus->i2c_port, daddr, 
                                               write_buf, 2, 
                                               pdMS_TO_TICKS(bus->timeout_ms));
    return (err == ESP_OK) ? 0 : 1;
}

/**
 * @brief I2C 写多字节
 */
uint8_t IIC_Write_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]) {
    uint8_t write_buf[length + 1];
    write_buf[0] = reg;
    memcpy(&write_buf[1], buff, length);

    esp_err_t err = i2c_master_write_to_device(bus->i2c_port, daddr, 
                                               write_buf, length + 1, 
                                               pdMS_TO_TICKS(bus->timeout_ms));
    return (err == ESP_OK) ? 0 : 1;
}

/**
 * @brief I2C 读单字节
 */
unsigned char IIC_Read_One_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg) {
    uint8_t data = 0;
    i2c_master_write_read_device(bus->i2c_port, daddr, 
                                 &reg, 1, 
                                 &data, 1, 
                                 pdMS_TO_TICKS(bus->timeout_ms));
    return data;
}

/**
 * @brief I2C 读多字节
 */
uint8_t IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]) {
    // 注意：LSM303 系列在读取多个字节时，通常需要在寄存器地址 reg 的最高位加 1 (reg | 0x80)
    // 这样芯片内部地址指针才会自动增加。原代码没加，如果读出来数据不对，请尝试 reg | 0x80
    esp_err_t err = i2c_master_write_read_device(bus->i2c_port, daddr, 
                                                 &reg, 1, 
                                                 buff, length, 
                                                 pdMS_TO_TICKS(bus->timeout_ms));
    return (err == ESP_OK) ? 0 : 1;
}

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
