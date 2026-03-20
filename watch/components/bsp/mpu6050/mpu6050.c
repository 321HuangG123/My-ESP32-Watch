#include "mpu6050.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

/**************************************************************************/
/*!
	@brief  initialize the iic port connect with MPU6050

	@param  NULL
*/
/**************************************************************************/
// --- 中断处理回调函数 ---
// 注意：该函数运行在 ISR 环境中，不能使用 printf 或耗时操作
uint8_t HardInt_mpu_flag=0;
static void IRAM_ATTR mpu_isr_handler(void* arg) 
{
    HardInt_mpu_flag=1;
}

void MPU_INT_Pin_Init(void)
{
    // 1. 配置 GPIO 参数
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MPU_INT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   // 开启上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,     // 下降沿触发 (对应 GPIO_MODE_IT_FALLING)
    };
    gpio_config(&io_conf);

    // 2. 安装全局 ISR 服务 (如果其他地方已经安装过，此函数会返回错误，可以忽略)
    // ESP_INTR_FLAG_LEVEL1 表示中断优先级，通常设为 0 即可
    gpio_install_isr_service(0);

    // 3. 为 MPU6050 的中断引脚绑定处理函数
    gpio_isr_handler_add(MPU_INT_GPIO, mpu_isr_handler, (void*) MPU_INT_GPIO);
}

/**************************************************************************/
/*!
	@brief  initialize the motion function of MPU6050

	@param  NULL
*/
/**************************************************************************/
void MPU_Motion_Init(void)
{
	MPU_Write_Byte(MPU_MOTION_DET_REG, 0x01); // set the acceleration threshold is (LSB*2)mg
	MPU_Write_Byte(MPU_MOTION_DUR_REG, 0x01); // Acceleration detection time is ()ms
	MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X90);  // INT Pin active low level, reset until 50us
	MPU_Write_Byte(MPU_INT_EN_REG, 0x40);	  // enable INT
}

/**************************************************************************/
/*!
	@brief  initialize the IIC bus

	@param  NULL
*/
/**************************************************************************/
void MPU_Bus_Init(void)
{
	{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          // 可选：指定时钟源
    };

    // 配置 I2C 参数
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        // 可以在这里加个串口打印报错
        return;
    }

    // 安装 I2C 驱动
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
}

/**************************************************************************/
/*!
	@brief  init the MPU6050

	@param  NULL

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Init(void)
{
	uint8_t res;

	MPU_Bus_Init();

	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X80); // 复位MPU6050
	vTaskDelay(pdMS_TO_TICKS(100));
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X00); // 唤醒MPU6050
	MPU_Set_Gyro_Fsr(3);					 // G传感器, 2000dps
	MPU_Set_Accel_Fsr(2);					 // A传感器, 8g
	MPU_Set_Rate(50);						 // 采样率50Hz
	MPU_Write_Byte(MPU_INT_EN_REG, 0X00);	 // 关闭所有中断
	MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // IIC主模式关闭
	MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00);	 // dis FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT active low

	res = MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if (res == MPU_ADDR) // ID
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X28); // SET the internal 8MHz,sleep=0,cycle=1,TEMP_DIS=1//low power modes
		MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X87); // enable accelerometer,disanable gyroscope,set the wake up frequence=20Hz
		MPU_Set_Rate(50);						 // 采样率50Hz
	}
	else
		return 1;

	MPU_Motion_Init();
	MPU_INT_Pin_Init();

	return 0;
}

void MPU_Sleep()
{
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x48); // sleep=1,cycle=0,temp_dis=1,internal 8MHz
}

void MPU_Wakeup()
{
	// low power modes
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x28); // sleep=0,cycle=1,temp_dis=1,internal 8MHz
}

uint8_t MPU_Read_Status()
{
	return MPU_Read_Byte(MPU_INT_STA_REG);
}

/**************************************************************************/
/*
	@brief  设置MPU6050陀螺仪传感器满量程范围

	@param  fsr:0,+250dps;1,500dps;2,+1000dps;3,+2000dps

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3);
}

/**************************************************************************/
/*
	@brief  设置MPU6050的数字低通滤波器

	@param  fsr:低通滤波器频率(Hz)

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3);
}

/**************************************************************************/
/*
	@brief  设置MPU6050的低通滤波器

	@param  lpf: Hz

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Set_LPF(uint16_t lpf)
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
	return MPU_Write_Byte(MPU_CFG_REG, data);
}

/**************************************************************************/
/*
	@brief  设置MPU6050的采样率

	@param  rate: 4~1000 Hz

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Set_Rate(uint16_t rate)
{
	uint8_t data;
	if (rate > 1000)
		rate = 1000;
	if (rate < 4)
		rate = 4;
	data = 1000 / rate - 1;
	data = MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data);
	return MPU_Set_LPF(rate / 2);
}

/**************************************************************************/
/*
	@brief  获取MPU6050温度值

	@param  NULL

	@return temperature (short)
*/
/**************************************************************************/
short MPU_Get_Temperature(void)
{
	uint8_t buf[2];
	short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
	raw = ((uint16_t)buf[0] << 8) | buf[1];
	temp = 36.53 + ((double)raw) / 340;
	return temp * 100;
	;
}

/**************************************************************************/
/*
	@brief  获取MPU6050陀螺仪原始值

	@param  NULL

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
	uint8_t buf[6], res;
	res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
	if (res == 0)
	{
		*gx = ((uint16_t)buf[0] << 8) | buf[1];
		*gy = ((uint16_t)buf[2] << 8) | buf[3];
		*gz = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return res;
	;
}

/**************************************************************************/
/*
	@brief  获取MPU6050加速度原始值

	@param  NULL

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
	uint8_t buf[6], res;
	res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
	if (res == 0)
	{
		*ax = ((uint16_t)buf[0] << 8) | buf[1];
		*ay = ((uint16_t)buf[2] << 8) | buf[3];
		*az = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return res;
	;
}

/**************************************************************************/
/*
	@brief  IIC连续写

	@param  addr:器件地址
	@param  reg:寄存器地址
	@param  len:写入长度
	@param  buf:数据区

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	// ESP-IDF 要求在一个 buffer 中发送 [寄存器地址, 数据1, 数据2...]
    uint8_t write_buf[len + 1];
    write_buf[0] = reg;
    memcpy(&write_buf[1], buf, len);

    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, addr, 
                                               write_buf, len + 1, 
                                               pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    
    return (err == ESP_OK) ? 0 : 1;
}

/**************************************************************************/
/*
	@brief  IIC写单字节

	@param  reg:寄存器地址
	@param  data:数据(uint8_t)

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data)
{
	return MPU_Write_Len(MPU_ADDR, reg, 1, &data);
}

/**************************************************************************/
/*
	@brief  IIC读单字节

	@param  reg:寄存器地址

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Read_Byte(uint8_t reg)
{
	uint8_t data = 0;
	MPU_Read_Len(MPU_ADDR, reg, 1, &data);
	return data;
}

/**************************************************************************/
/*
	@brief  IIC连续读

	@param  addr:器件地址
	@param  reg:寄存器地址
	@param  len:写入长度
	@param  buf:数据区

	@return 0 if success
*/
/**************************************************************************/
uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	// 先写入寄存器地址，然后立即切换为读取模式
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, addr, 
                                                 &reg, 1, 
                                                 buf, len, 
                                                 pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    
    return (err == ESP_OK) ? 0 : 1;
}

// uint8_t MPU_Write_Multi_Byte(uint8_t addr, uint8_t length, uint8_t buff[])
// {
// 	if (IIC_Write_Multi_Byte(&MPU_bus, MPU_ADDR << 1, addr, length, buff))
// 	{
// 		return 1;
// 	}
// 	return 0;
// }

// uint8_t MPU_Read_Multi_Byte(uint8_t addr, uint8_t length, uint8_t buff[])
// {
// 	if (IIC_Read_Multi_Byte(&MPU_bus, MPU_ADDR << 1, addr, length, buff))
// 	{
// 		return 1;
// 	}
// 	return 0;
// }

/**************************************************************************/
/*
	@brief  get the roll and pitch

	@param  roll:  横滚角(float)
	@param  pitch: 俯仰角(float)

	@return NULL
*/
/**************************************************************************/
void MPU_Get_Angles(float *roll, float *pitch)
{
	short ax, ay, az;
	MPU_Get_Accelerometer(&ax, &ay, &az);
	*pitch = -atanf(ax / sqrtf(ay * ay + az * az));
	*roll = atanf((float)ay / (float)az);
}

/**************************************************************************/
/*
	@brief  check the MPU6050 is horizontal or not

	@param  NULL

	@return 1 if is horizontal
*/
/**************************************************************************/
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
