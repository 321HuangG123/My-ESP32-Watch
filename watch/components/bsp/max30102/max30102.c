/**
 * @file max30102.c
 * @brief MAX30102心率血氧传感器驱动实现
 * @details 实现通过I2C总线与MAX30102通信，配置和读取心率血氧数据
 *
 */

#include "max30102.h"
#include "driver/i2c.h"
#include "esp_err.h"

/** MAX30102 I2C设备地址 */
#define MAX30102_WR_ADDRESS 0xAE  // 写地址

/**
 * @brief 向MAX30102寄存器写入数据
 * @param uch_addr 寄存器地址
 * @param uch_data 要写入的数据
 * @return true:成功; false:失败
 */
bool maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
{
    // 1. 获取句柄
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // 2. 发送起始信号
    i2c_master_start(cmd);

    // 3. 发送设备地址和写控制位
    i2c_master_write_byte(cmd, MAX30102_WR_ADDRESS | I2C_WR, true); 

    // 4. 发送寄存器地址
    i2c_master_write_byte(cmd, uch_addr, true); 

    // 5. 发送要写入寄存器的数据
    i2c_master_write_byte(cmd, uch_data, true); 

    // 6. 发送停止信号
    i2c_master_stop(cmd);

    // 7. 执行命令，设置超时时间为10ms
    esp_err_t res = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
    
    i2c_cmd_link_delete(cmd);

    return (res == ESP_OK);
}

/**
 * @brief 从MAX30102寄存器读取数据
 * @param uch_addr 寄存器地址
 * @param puch_data 读取数据存储指针
 * @return true:成功; false:失败
 */
bool maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    /* 第1步：发起I2C总线启动信号 */
    i2c_master_start(cmd);

    /* 第2步：发送设备地址和写控制位(先写入要读取的寄存器地址) */
    i2c_master_write_byte(cmd, MAX30102_WR_ADDRESS | I2C_WR, true);

    /* 第3步：发送寄存器地址 */
    i2c_master_write_byte(cmd, uch_addr, true);

    /* 第4步：重新启动I2C总线，准备读取数据 */
    i2c_master_start(cmd);

    /* 第5步：发送设备地址和读控制位 */
    i2c_master_write_byte(cmd, MAX30102_WR_ADDRESS | I2C_RD, true);

    /* 第6步：读取寄存器数据 */
    i2c_master_read_byte(cmd, puch_data, I2C_MASTER_NACK);

    /* 第7步：发送I2C总线停止信号 */
    i2c_master_stop(cmd);

    // 执行命令，设置超时时间为10ms
    esp_err_t res = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
    
    i2c_cmd_link_delete(cmd);

    return (res == ESP_OK);

}

/**
 * @brief 初始化MAX30102传感器
 * @return true:初始化成功; false:初始化失败
 * @note 配置传感器工作模式、采样率、LED电流等参数
 */
bool maxim_max30102_init(void)
{
    /* 配置中断使能寄存器 */
    if(!maxim_max30102_write_reg(REG_INTR_ENABLE_1, 0xC0))  // 0xC0: 只使能FIFO满和数据就绪中断
        return false;
    if(!maxim_max30102_write_reg(REG_INTR_ENABLE_2, 0x00))  // 禁用温度就绪中断
        return false;
    
    /* 配置FIFO寄存器 */
    if(!maxim_max30102_write_reg(REG_FIFO_WR_PTR, 0x00))    // 重置FIFO写指针
        return false;
    if(!maxim_max30102_write_reg(REG_OVF_COUNTER, 0x00))    // 清零溢出计数器
        return false;
    if(!maxim_max30102_write_reg(REG_FIFO_RD_PTR, 0x00))    // 重置FIFO读指针
        return false;
    
    /* FIFO配置: 样本平均数=8, 禁用溢出回滚, FIFO满阈值=17 */
    if(!maxim_max30102_write_reg(REG_FIFO_CONFIG, 0x6F))
        return false;
    
    /* 模式配置: SpO2模式 (心率+血氧) */
    if(!maxim_max30102_write_reg(REG_MODE_CONFIG, 0x03))
        return false;
    
    /* SpO2配置: ADC量程=4096nA, 采样率=400Hz, LED脉冲宽度=411μs */
    if(!maxim_max30102_write_reg(REG_SPO2_CONFIG, 0x2F))
        return false;

    /* 配置LED驱动电流 */
    if(!maxim_max30102_write_reg(REG_LED1_PA,  0x24))  // LED1( 红光 )电流
        return false;
    if(!maxim_max30102_write_reg(REG_LED2_PA,  0x24))  // LED2(红外光)电流
        return false;
    if(!maxim_max30102_write_reg(REG_PILOT_PA, 0x7F))  // 导航LED电流~25mA
        return false;
    
    return true;  // 所有配置成功
}

/**
 * @brief 从MAX30102 FIFO寄存器读取一组样本数据
 * @param pun_red_led 指向存储红光LED读数的变量指针
 * @param pun_ir_led 指向存储红外光LED读数的变量指针
 * @return true:成功读取; false:读取失败
 */
bool maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)
{
    uint8_t data_buf[6];        // 准备一个6字节的缓冲区存放原始数据
    uint8_t uch_temp;
    
    /* 初始化读取值 */
    *pun_ir_led = 0;
    *pun_red_led = 0;
    
    /* 读取并清除中断状态 */
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
    maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);

    /* 开始读取FIFO数据 */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // 起始 --> 发送写地址 --> 发送寄存器地址
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MAX30102_WR_ADDRESS | I2C_WR, true);
    i2c_master_write_byte(cmd, (uint8_t)REG_FIFO_DATA, true);

    // 重复起始信号 --> 发送读地址
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MAX30102_WR_ADDRESS | I2C_RD, true);
    
    /* 读取红光LED数据 (3字节) */
    /* 读取红外光LED数据 (3字节) */
    // 一次性读取 6 个字节数据到data_buf中
    i2c_master_read(cmd, data_buf, 6, I2C_MASTER_LAST_NACK);

    // 发送停止信号
    i2c_master_stop(cmd);

    // 执行命令，设置超时时间为10ms
    esp_err_t res = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(cmd);

    // 发送成功后再解析数据
    if (res == ESP_OK)
    {
        // 解析红光LED数据 前三字节
        *pun_red_led = ((uint32_t)data_buf[0] << 16) | ((uint32_t)data_buf[1] << 8) | ((uint32_t)data_buf[2] << 0);
        *pun_red_led &= 0x03FFFF;   // 18位掩码

        // 解析红外光 LED 数据 (后 3 字节)
        *pun_ir_led  = ((uint32_t)data_buf[3] << 16) | ((uint32_t)data_buf[4] << 8) | ((uint32_t)data_buf[5] << 0);
        *pun_ir_led &= 0x03FFFF; // 18位掩码
        return true;
    }
    
    return false;

}

/**
 * @brief 重置MAX30102传感器
 * @return true:重置成功; false:重置失败
 * @note 向模式配置寄存器写入重置位(0x40)，触发软件重置
 */
bool maxim_max30102_reset(void)
{
    return maxim_max30102_write_reg(REG_MODE_CONFIG, 0x40);
}

/**
 * @brief 读取MAX30102的设备ID
 * @param id 存储设备ID的指针
 * @return true:读取成功; false:读取失败
 */
bool maxim_max30102_read_id(uint8_t *id)
{
    return maxim_max30102_read_reg(REG_PART_ID, id);
}

/**
 * @brief 清空MAX30102 FIFO缓冲区
 * @return true:成功; false:失败
 * @note 清空FIFO的方法是将读写指针都设为相同值
 */
bool maxim_max30102_clear_fifo(void)
{
    if(!maxim_max30102_write_reg(REG_FIFO_WR_PTR, 0x00))
        return false;
    if(!maxim_max30102_write_reg(REG_OVF_COUNTER, 0x00))
        return false;
    if(!maxim_max30102_write_reg(REG_FIFO_RD_PTR, 0x00))
        return false;
    
    return true;
}

/**
 * @brief 启用或禁用MAX30102的低功耗模式
 * @param enable true:启用低功耗; false:禁用低功耗
 * @return true:设置成功; false:设置失败
 */
bool maxim_max30102_set_low_power(bool enable)
{
    uint8_t reg_value;
    
    if(!maxim_max30102_read_reg(REG_MODE_CONFIG, &reg_value))
        return false;
    
    if(enable)
        reg_value |= 0x20;  // 设置低功耗模式位
    else
        reg_value &= ~0x20; // 清除低功耗模式位
    
    return maxim_max30102_write_reg(REG_MODE_CONFIG, reg_value);
}

void MAX30102_hrs_DisEnable(void)
{
    // 进入低功耗模式
    maxim_max30102_set_low_power(true);
}