#include "i2c_u.h"


// 写入一个字节
uint8_t IIC_Write_One_Byte(esp32_i2c_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    esp_err_t err = i2c_master_write_to_device(bus->port, daddr, write_buf, 2, pdMS_TO_TICKS(100));
    return (err == ESP_OK) ? 0 : 1;
}

// 读取一个字节
unsigned char IIC_Read_One_Byte(esp32_i2c_bus_t *bus, uint8_t daddr, uint8_t reg) {
    uint8_t data = 0;
    // 注意：daddr 传入 0x76 即可，无需左移
    esp_err_t err = i2c_master_write_read_device(bus->port, daddr, &reg, 1, &data, 1, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        return 0; // 或者返回一个错误码
    }
    return data;
}

// 初始化硬件 I2C
void IICInit(esp32_i2c_bus_t *bus) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = bus->sda_io,
        .scl_io_num = bus->scl_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000, // 400KHz
    };
    i2c_param_config(bus->port, &conf);
    i2c_driver_install(bus->port, conf.mode, 0, 0, 0);
}

