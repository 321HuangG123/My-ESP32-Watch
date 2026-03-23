#include "adc_power.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/**
 *********************************************************************************
 * @attention
 *
 * ADC 通道选择建议
 * ESP32 有 ADC1 和 ADC2：
 * ADC1：推荐使用（GPIO 1-10）。
 * ADC2：千万不要用来测电池。因为 ADC2 与 Wi-Fi 模块共用硬件，一旦你开启 Wi-Fi 联网校时，ADC2 就会失效，导致程序报错。
 *
 *********************************************************************************
 */


// 假设你使用的是 ADC1 的通道 0 (GPIO 1)
#define BAT_ADC_CHAN          ADC_CHANNEL_0 
#define ADC_ATTEN             ADC_ATTEN_DB_12  // 12dB 衰减，可测量 0~3.3V

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle = NULL;

// 初始化 ADC (在硬件初始化任务中调用一次)
void Power_ADC_Init(adc_channel_t channel) {
    // 1. 创建 ADC 单元
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    // 2. 配置通道
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // S3 默认为 12位 (4096)
        .atten = ADC_ATTEN,
    };
    adc_oneshot_config_channel(adc1_handle, channel, &config);

    // 3. 关键：校准初始化 (ESP32 必须要做这一步)
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle);
}

static float BatCheck_8times(void) {
    int adc_raw[8];
    int total_raw = 0;
    int voltage_ms = 0; // 这里的单位是 毫伏 (mV)

    for (int i = 0; i < 8; i++) {
        // 读取原始值
        adc_oneshot_read(adc1_handle, BAT_ADC_CHAN, &adc_raw[i]);
        total_raw += adc_raw[i];
        vTaskDelay(pdMS_TO_TICKS(1)); // 替代 delay_ms(1)
    }
    
    int avg_raw = total_raw >> 3; // 取平均值

    // 使用校准句柄将原始值转换为 毫伏
    if (adc1_cali_handle) {
        adc_cali_raw_to_voltage(adc1_cali_handle, avg_raw, &voltage_ms);
    } else {
        // 如果校准失败，才使用 STM32 那种手动换算公式
        voltage_ms = (avg_raw * 3300) / 4095;
    }

    // 原项目中分压电路通常是 1/2 分压 (R1=10K, R2=10K)
    // 所以结果要乘以 2，再除以 1000 换算成 伏特 (V)
    return (float)voltage_ms * 2.0 / 1000.0;
}

/**
 * @brief	利用ADC计算剩余电量
 * @param   NULL
 * @return  剩余电量百分比值
 */
uint8_t PowerCalculate(void)
{
    uint8_t power = 0;
	float voltage;
	voltage = BatCheck_8times();

    // 如果是在充电，就考虑特殊情况，但我们这里没有实际安上电池，所以不考虑，大概写一下逻辑即可
	// if (ChargeCheck())
	// {
	// 	voltage -= INTERNAL_RES * CHARGING_CUR;
	// }

	// 根据电压值映射剩余电量百分比大概数值是在多少
	if ((voltage >= 4.2))
	{
		power = 100;
	}
	else if (voltage >= 4.06 && voltage < 4.2)
	{
		power = 90;
	}
	else if (voltage >= 3.98 && voltage < 4.06)
	{
		power = 80;
	}
	else if (voltage >= 3.92 && voltage < 3.98)
	{
		power = 70;
	}
	else if (voltage >= 3.87 && voltage < 3.92)
	{
		power = 60;
	}
	else if (voltage >= 3.82 && voltage < 3.87)
	{
		power = 50;
	}
	else if (voltage >= 3.79 && voltage < 3.82)
	{
		power = 40;
	}
	else if (voltage >= 3.77 && voltage < 3.79)
	{
		power = 30;
	}
	else if (voltage >= 3.74 && voltage < 3.77)
	{
		power = 20;
	}
	else if (voltage >= 3.68 && voltage < 3.74)
	{
		power = 10;
	}
	else if (voltage >= 3.45 && voltage < 3.68)
	{
		power = 5;
	}
	return power;
}
