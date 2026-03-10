/*******************************************************************************
  * @file    max30102_read.c
  * @author  MCD Application Team
  * @brief   This file provides MAX30102 read driver functions.
  ******************************************************************************
  * @attention
  */
#include "max30102_read.h"
#include "max30102.h"
#include "max30102_algorithm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define TAG "MAX30102_READ"

#define MAX_BRIGHTNESS 255   



uint32_t aun_ir_buffer[150];   
uint32_t aun_red_buffer[150];  
int32_t n_ir_buffer_length;    
int32_t n_spo2;                
int8_t ch_spo2_valid;          
int32_t n_heart_rate;
int8_t ch_hr_valid;            
uint8_t uch_dummy;             


int32_t hr_buf[16];            
int32_t hrSum;                 
extern int32_t hrAvg;          
int32_t hrBuffFilled;          
int32_t hrValidCnt = 0;        
int32_t hrThrowOutSamp = 0;    
int32_t hrTimeout = 0;         


int32_t spo2_buf[16];          
int32_t spo2Sum;               
extern int32_t spo2Avg;        
int32_t spo2BuffFilled;        
int32_t spo2ValidCnt = 0;      
int32_t spo2ThrowOutSamp = 0;  
int32_t spo2Timeout = 0;       


uint32_t un_min, un_max, un_prev_data;  
uint32_t un_brightness;                 


/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define I2C_MASTER_SCL_IO           GPIO_NUM_5
#define I2C_MASTER_SDA_IO           GPIO_NUM_4
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

/*
*********************************************************************************************************
*	函 数 名: bsp_InitI2C
*	功能说明: 配置I2C总线的GPIO，
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitI2C(void)
{
	i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/*
*********************************************************************************************************
*	函 数 名: i2c_CheckDevice
*	功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*	形    参:  _Address：设备的I2C总线地址
*	返 回 值: 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // 注意：硬件驱动通常需要传入7位地址，这里假设 _Address 是已经左移过的8位地址
    i2c_master_write_byte(cmd, _Address | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);

    return (ret == ESP_OK) ? 0 : 1;
}

uint8_t Init_MAX30102(void)
{
	int32_t i;

	un_brightness = 0;
	un_min = 0x3FFFF;  
	un_max = 0;        
	
	bsp_InitI2C();
	bool res;
	res = maxim_max30102_reset();  
	vTaskDelay(pdMS_TO_TICKS(200)); 	// 延时200ms保证重启成功  
	if (res == false)
	{
		ESP_LOGI(TAG, "max30102 reset faild...");
		return 1;
	}
	                        
	maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy);

	maxim_max30102_init(); 
	vTaskDelay(pdMS_TO_TICKS(200)); 	// 延时200ms保证初始化成功  
	if (res == false)
	{
		ESP_LOGI(TAG, "max30102 init faild...");
		return 1;
	}                              

	n_ir_buffer_length = 150;

	for(i = 0; i < n_ir_buffer_length; i++)
	{
		// // 延时等待，确保FIFO里有数据
		vTaskDelay(pdMS_TO_TICKS(30));

		maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));

		if(un_min > aun_red_buffer[i])
			un_min = aun_red_buffer[i];  
		if(un_max < aun_red_buffer[i])
			un_max = aun_red_buffer[i];  
	}
	un_prev_data = aun_red_buffer[i];

	
	maxim_heart_rate_and_oxygen_saturation(
		aun_ir_buffer,
		n_ir_buffer_length,
		aun_red_buffer,
		&n_spo2,
		&ch_spo2_valid,
		&n_heart_rate,
		&ch_hr_valid
	);

	return 0;
}


void ReadHeartRateSpO2(void)
{
	int32_t i;
	float f_temp;
	static uint8_t COUNT = 8; 
	

	i = 0;
	un_min = 0x3FFFF;
	un_max = 0;

	// 移动缓冲区数据
    // 我们假设我们要更新后50个数据：
    // 把 [50] 到 [149] 移到 [0] 到 [99]
    for(i = 50; i < 150; i++) {
        aun_red_buffer[i - 50] = aun_red_buffer[i];
        aun_ir_buffer[i - 50] = aun_ir_buffer[i];
		
		// 更新最小值，有助于后续亮度调节
		if(un_min > aun_red_buffer[i - 50])
			un_min = aun_red_buffer[i - 50];
		if(un_max < aun_red_buffer[i - 50])
			un_max = aun_red_buffer[i - 50];
    }
	

	// 读取50个样本
	for(i = 100; i < 150; i++)
	{
		// 等待数据生成 	50Hz采样率 = 20ms周期
		vTaskDelay(pdMS_TO_TICKS(20));

		// 读取前一个数据用于亮度自动调节逻辑
		un_prev_data = aun_red_buffer[i - 1];  
		
		maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));

		ESP_LOGI(TAG, "HR: %d, HR_Valid: %d, SpO2: %d, SpO2_Valid: %d", 
              n_heart_rate, ch_hr_valid, n_spo2, ch_spo2_valid);
		if(aun_red_buffer[i] > un_prev_data)  
		{
			f_temp = aun_red_buffer[i] - un_prev_data;
			f_temp /= (un_max - un_min);           
			f_temp *= MAX_BRIGHTNESS;              
			f_temp = un_brightness - f_temp;       
			if(f_temp < 0)
				un_brightness = 0;
			else
				un_brightness = (int)f_temp;
		}
		else  
		{
			f_temp = un_prev_data - aun_red_buffer[i];
			f_temp /= (un_max - un_min);           
			f_temp *= MAX_BRIGHTNESS;              
			un_brightness += (int)f_temp;          
			if(un_brightness > MAX_BRIGHTNESS)
				un_brightness = MAX_BRIGHTNESS;
		}
	}

	
	maxim_heart_rate_and_oxygen_saturation(
		aun_ir_buffer,
		n_ir_buffer_length,
		aun_red_buffer,
		&n_spo2,
		&ch_spo2_valid,
		&n_heart_rate,
		&ch_hr_valid
	);

	if(COUNT++ > 8)
	{
		COUNT = 0;
		
		if ((ch_hr_valid == 1) && (n_heart_rate < 150) && (n_heart_rate > 60))  
		{
			hrTimeout = 0;  

			if (hrValidCnt == 4)
			{
				hrThrowOutSamp = 1;  
				hrValidCnt = 0;
				
				for (i = 12; i < 16; i++)
				{
					if (n_heart_rate < hr_buf[i] + 10)  
					{
						hrThrowOutSamp = 0;
						hrValidCnt   = 4;
					}
				}
			}
			else
			{
				hrValidCnt = hrValidCnt + 1;  
			}

			if (hrThrowOutSamp == 0)
			{
				for(i = 0; i < 15; i++)
				{
					hr_buf[i] = hr_buf[i + 1];  
				}
				hr_buf[15] = n_heart_rate;     

				if (hrBuffFilled < 16)
				{
					hrBuffFilled = hrBuffFilled + 1;
				}
				hrSum = 0;
				if (hrBuffFilled < 2) 
				{
					hrAvg = n_heart_rate;
				}
				else if (hrBuffFilled < 4)  
				{
					for(i = 14; i < 16; i++)
					{
						hrSum = hrSum + hr_buf[i];
					}
					hrAvg = hrSum >> 1;  
				}
				else if (hrBuffFilled < 8)  
				{
					for(i = 12; i < 16; i++)
					{
						hrSum = hrSum + hr_buf[i];
					}
					hrAvg = hrSum >> 2;  
				}
				else if (hrBuffFilled < 16)  
				{
					for(i = 8; i < 16; i++)
					{
						hrSum = hrSum + hr_buf[i];
					}
					hrAvg = hrSum >> 3;  
				}
				else  
				{
					for(i = 0; i < 16; i++)
					{
						hrSum = hrSum + hr_buf[i];
					}
					hrAvg = hrSum >> 4;  
				}
			}
			hrThrowOutSamp = 0; 
		}
		else  
		{
			hrValidCnt = 0;
			if (hrTimeout == 8)  
			{
				hrAvg = 0;         
				hrBuffFilled = 0;  
			}
			else
			{
				hrTimeout++;  
			}
		}

		
		if ((ch_spo2_valid == 1) && (n_spo2 > 80))  
		{
			spo2Timeout = 0;  

			
			if (spo2ValidCnt == 4)
			{
				spo2ThrowOutSamp = 1;  
				spo2ValidCnt = 0;
				
				
				for (i = 12; i < 16; i++)
				{
					if (n_spo2 > spo2_buf[i] - 10)  
					{
						spo2ThrowOutSamp = 0;
						spo2ValidCnt   = 4;
					}
				}
			}
			else
			{
				spo2ValidCnt = spo2ValidCnt + 1;  
			}

			
			if (spo2ThrowOutSamp == 0)
			{
				
				for(i = 0; i < 15; i++)
				{
					spo2_buf[i] = spo2_buf[i + 1]; 
				}
				spo2_buf[15] = n_spo2;              

				
				if (spo2BuffFilled < 16)
				{
					spo2BuffFilled = spo2BuffFilled + 1;
				}

				
				spo2Sum = 0;
				if (spo2BuffFilled < 2)  
				{
					spo2Avg = n_spo2;
				}
				else if (spo2BuffFilled < 4)  
				{
					for(i = 14; i < 16; i++)
					{
						spo2Sum = spo2Sum + spo2_buf[i];
					}
					spo2Avg = spo2Sum >> 1;  
				}
				else if (spo2BuffFilled < 8)  
				{
					for(i = 12; i < 16; i++)
					{
						spo2Sum = spo2Sum + spo2_buf[i];
					}
					spo2Avg = spo2Sum >> 2;  
				}
				else if (spo2BuffFilled < 16)  
				{
					for(i = 8; i < 16; i++)
					{
						spo2Sum = spo2Sum + spo2_buf[i];
					}
					spo2Avg = spo2Sum >> 3;  
				}
				else  
				{
					for(i = 0; i < 16; i++)
					{
						spo2Sum = spo2Sum + spo2_buf[i];
					}
					spo2Avg = spo2Sum >> 4;  
				}
			}
			spo2ThrowOutSamp = 0; 
		}
		else 
		{
			spo2ValidCnt = 0;
			if (spo2Timeout == 8)  
			{
				spo2Avg = 0;         
				spo2BuffFilled = 0;  
			}
			else
			{
				spo2Timeout++;  
			}
		}
	}
}