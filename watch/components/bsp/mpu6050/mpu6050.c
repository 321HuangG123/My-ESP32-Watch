#include "mpu6050.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"

#define TAG "MPU6050"

#define MPU6050_I2C_NUM I2C_NUM_0
#define MPU_I2C_SCL GPIO_NUM_0
#define MPU_I2C_SDA GPIO_NUM_1
#define MPU_INT GPIO_NUM_12

/* ESP32 I2C 连续写多个字节 */
uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write(cmd, buf, len, true);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(MPU6050_I2C_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    return (err == ESP_OK) ? 0 : 1;
}

/* ESP32 I2C 连续读多个字节 */
uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf) {
    if (len == 0) return 0;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(MPU6050_I2C_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    return (err == ESP_OK) ? 0 : 1;
}

/* 包装成官方库偶尔会用到的单字节读写 */
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data) {
    return MPU_Write_Len(MPU6050_ADDR, reg, 1, &data);
}

uint8_t MPU_Read_Byte(uint8_t reg) {
    uint8_t res;
    MPU_Read_Len(MPU6050_ADDR, reg, 1, &res);
    return res;
}

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    uint8_t res;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // 创建链接，装载容器

    i2c_master_start(cmd);                                                    // 产生起始信号
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true); // 发送从机地址，并产生应答
    i2c_master_write_byte(cmd, RegAddress, true);                             // 发送从机数据寄存器的地址，并产生应答
    i2c_master_write_byte(cmd, Data, true);                                   // 写入数据 并产生应答
    i2c_master_stop(cmd);                                                     // 产生停止信号
    res = i2c_master_cmd_begin(MPU6050_I2C_NUM, cmd, pdMS_TO_TICKS(100));     // 启动容器，开始工作

    if (res == ESP_OK)
    {
        ESP_LOGI(TAG, "MPU6050_WriteReg success - RegAddress: 0x%02X, Data: 0x%02X", RegAddress, Data);
    }
    else
    {
        ESP_LOGE(TAG, "MPU6050_WriteReg failed with error: %d - RegAddress: 0x%02X, Data: 0x%02X", res, RegAddress, Data);
    }

    i2c_cmd_link_delete(cmd); // 删除链接容器，避免占用资源
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
    uint8_t Data = 0;
    uint8_t res;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                             // 创建链接，装载容器
    i2c_master_start(cmd);                                                    // 产生起始信号
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true); // 发送从机地址，并产生应答
    i2c_master_write_byte(cmd, RegAddress, true);                             // 发送从机数据寄存器的地址，并产生应答

    // 开始在该寄存器下读数据
    i2c_master_start(cmd);                                                   // 产生起始信号
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true); // 发送从机地址，读写位为1，表示即将读取，并产生应答
    i2c_master_read_byte(cmd, &Data, I2C_MASTER_LAST_NACK);                  // 读一个字节的数据至Data内，并且非应答
    i2c_master_stop(cmd);                                                    // 发送停止信号
    res = i2c_master_cmd_begin(MPU6050_I2C_NUM, cmd, pdMS_TO_TICKS(100));    // 启动容器，开始工作
    i2c_cmd_link_delete(cmd);                                                // 删除链接，保证资源不会被一直占用

    if (res == ESP_OK)
    {
        ESP_LOGI(TAG, "MPU6050_ReadReg success - RegAddress: 0x%02X, Data: 0x%02X", RegAddress, Data);
    }
    else
    {
        ESP_LOGE(TAG, "MPU6050_ReadReg failed with error: %d - RegAddress: 0x%02X, retrying...", res, RegAddress);
        vTaskDelay(10 / portTICK_PERIOD_MS); // 延迟10ms后重试
    }

    return Data;
}

// MPU中断处理函数
static void MPU_isr_handler(void *arg)
{
}


void MPU_Set_LPF(uint16_t lpf)
{
    uint8_t data = 0;
    if (lpf >= 188)
        data = 1;
    else if (lpf >= 98)
        data = 2;
    else if (lpf >= 42)
        data = 3;
    else if (lpf >= 20)
        data = 4;
    else if (lpf >= 10)
        data = 5;
    else
        data = 6;
    MPU6050_WriteReg(MPU_CFG_REG, data);
}


void MPU_Set_Rate(uint16_t rate)
{

    uint8_t data;
    if (rate > 1000)
        rate = 1000;
    if (rate < 4)
        rate = 4;
    data = 1000 / rate - 1;
    MPU6050_WriteReg(MPU_SAMPLE_RATE_REG, data);
    MPU_Set_LPF(rate / 2);
}

void MPU6050_init(void)
{
    ESP_LOGI(TAG, "MPU6050 I2C Init......");

    // I2C 配置
    i2c_config_t i2c_cfg = {
        .clk_flags = 0, // 采用默认时钟
        .scl_io_num = MPU_I2C_SCL,
        .sda_io_num = MPU_I2C_SDA,
        .mode = I2C_MODE_MASTER,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 50 * 1000, // 通信速率
    };
    ESP_ERROR_CHECK(i2c_param_config(MPU6050_I2C_NUM, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(MPU6050_I2C_NUM, i2c_cfg.mode, 0, 0, 0));

    MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0x80); // 重置MPU6050
    vTaskDelay(pdMS_TO_TICKS(500));

    MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0X00); // 唤醒MPU6050
    MPU6050_WriteReg(MPU_GYRO_CFG_REG, 0x03);  // G传感器, 2000dps
    MPU6050_WriteReg(MPU_ACCEL_CFG_REG, 0x02); // A传感器, 8g
    MPU_Set_Rate(50);                          // 采样率50Hz
    MPU6050_WriteReg(MPU_INT_EN_REG, 0X00);    // 关闭所有中断
    MPU6050_WriteReg(MPU_USER_CTRL_REG, 0X00); // IIC主模式关闭
    MPU6050_WriteReg(MPU_FIFO_EN_REG, 0X00);   // 关闭 FIFO 使能
    MPU6050_WriteReg(MPU_INTBP_CFG_REG, 0X80); // 中断时 INT 引脚拉低

    MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0X28); // 内部时钟 8MHz；sleep=0,cycle=1（睡眠模式和唤醒模式之间循环）；TEMP_DIS=1禁用温度传感器 降低功耗
    MPU6050_WriteReg(MPU_PWR_MGMT2_REG, 0X87); // 使能加速度，禁用陀螺仪，唤醒频率 5Hz
    MPU_Set_Rate(50);                          // 采样率50Hz

    MPU6050_WriteReg(MPU_MOTION_DET_REG, 0x30); // 加速度阈值 48mg
    MPU6050_WriteReg(MPU_MOTION_DUR_REG, 0x0A); // 加速度测量时间 10ms
    MPU6050_WriteReg(MPU_INTBP_CFG_REG, 0X90);  // 中断时 INT 引脚拉低，中断状态位在任何读操作中都被清除。
    MPU6050_WriteReg(MPU_INT_EN_REG, 0x40);     // 开启 运动状态检测 中断

    // 中断 gpio口配置
    gpio_config_t int_cfg = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pin_bit_mask = (1ull << MPU_INT),
    };
    gpio_config(&int_cfg);
    gpio_intr_enable(MPU_INT);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL2);
    gpio_isr_handler_add(MPU_INT, MPU_isr_handler, NULL);
}

void MPU6050_GetGyroXData(int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t DataH, DataL;

    DataH = MPU6050_ReadReg(MPU_GYRO_XOUTH_REG); // 读取陀螺仪X轴的高8位数据
    DataL = MPU6050_ReadReg(MPU_GYRO_XOUTL_REG); // 读取陀螺仪X轴的低8位数据
    *GyroX = (DataH << 8) | DataL;
    ESP_LOGI(TAG, "MPU6050 get GyroX : [%d] ", *GyroX);

    DataH = MPU6050_ReadReg(MPU_GYRO_YOUTH_REG); // 读取陀螺仪Y轴的高8位数据
    DataL = MPU6050_ReadReg(MPU_GYRO_YOUTL_REG); // 读取陀螺仪Y轴的低8位数据
    *GyroY = (DataH << 8) | DataL;
    ESP_LOGI(TAG, "MPU6050 get GyroY : [%d] ", *GyroY);

    DataH = MPU6050_ReadReg(MPU_GYRO_ZOUTH_REG); // 读取陀螺仪Z轴的高8位数据
    DataL = MPU6050_ReadReg(MPU_GYRO_ZOUTL_REG); // 读取陀螺仪Z轴的低8位数据
    *GyroZ = (DataH << 8) | DataL;
    ESP_LOGI(TAG, "MPU6050 get GyroZ : [%d] ", *GyroZ);
}

void MPU6050_GetAccxData(int16_t *AccX, int16_t *AccY, int16_t *AccZ)
{
    uint8_t DataH, DataL;

    DataH = MPU6050_ReadReg(MPU_ACCEL_XOUTH_REG); // 读取加速度x轴寄存器的高八位
    DataL = MPU6050_ReadReg(MPU_ACCEL_XOUTL_REG); // 读取加速度x轴寄存器的低八位
    *AccX = (DataH << 8) | DataL;
    ESP_LOGI(TAG, "MPU6050 get AccX : [%d] ", *AccX);

    DataH = MPU6050_ReadReg(MPU_ACCEL_YOUTH_REG); // 读取加速度y轴寄存器的高八位
    DataL = MPU6050_ReadReg(MPU_ACCEL_YOUTL_REG); // 读取加速度y轴寄存器的低八位
    *AccY = (DataH << 8) | DataL;
    ESP_LOGI(TAG, "MPU6050 get AccY : [%d] ", *AccY);

    DataH = MPU6050_ReadReg(MPU_ACCEL_ZOUTH_REG); // 读取加速度z轴寄存器的高八位
    DataL = MPU6050_ReadReg(MPU_ACCEL_ZOUTL_REG); // 读取加速度z轴寄存器的低八位
    *AccZ = (DataH << 8) | DataL;
    ESP_LOGI(TAG, "MPU6050 get AccZ : [%d] ", *AccZ);
}

uint8_t MPU6050_GetID(void)
{
    uint8_t id = MPU6050_ReadReg(MPU_WHO_AM_I_REG);
    ESP_LOGI(TAG, "MPU6050 ID : [%02X]", id);
    return id;
}

void MPU_Sleep()
{
    MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0x48); // sleep=1,cycle=0,temp_dis=1,internal 8MHz
}

void MPU_Wakeup()
{
    // low power modes
    MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0x28); // sleep=0,cycle=1,temp_dis=1,internal 8MHz
}

uint8_t MPU_Read_Status()
{
    return MPU6050_ReadReg(MPU_INT_STA_REG);
}

uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr)
{
    return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3);
}

uint8_t MPU_Set_Accel_Fsr(uint8_t fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3);
}

short MPU_Get_Temperature(void)
{
    uint8_t buf[2];
    short raw;
    float temp;
    MPU_Read_Len(MPU6050_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
    raw = ((uint16_t)buf[0] << 8) | buf[1];
    temp = 36.53 + ((double)raw) / 340;
    return temp * 100;
}


void MPU_Get_Angles(float *roll, float *pitch)
{
    short ax, ay, az;
    MPU_Get_Accelerometer(&ax, &ay, &az);
    *pitch = -atanf(ax / sqrtf(ay * ay + az * az));
    *roll = atanf((float)ay / (float)az);
}


uint8_t MPU_isHorizontal(void)
{
    float roll, pitch;
    MPU_Get_Angles(&roll, &pitch);
    if (roll <= 0.50 && roll >= -0.50 && pitch <= 0.50 && pitch >= -0.50)
    {
        return 1;
    }
    return 0;
}
