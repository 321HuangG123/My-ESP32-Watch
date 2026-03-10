#include "pwm.h"
#include "driver/ledc.h"
#include "esp_err.h"


void init_pwm_on_gpiox(ledc_timer_config_t timer_conf, ledc_channel_config_t channel_conf)
{
    // 配置控制器定时器
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    // 配置控制器通道
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
}

void LCD_Set_Light(uint8_t dc)
{
    if (dc >= 10 && dc <= 100)
    {
        // 计算占空比
        uint32_t duty = (8191 * dc) / 100;  // 13位分辨率，最大值为8191

        // 设置新的占空比
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    }
    
}
