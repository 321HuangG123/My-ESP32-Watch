#ifndef __PWM_H__
#define __PWM_H__

#include "driver/ledc.h"

// 初始化pwm
void init_pwm_on_gpiox(ledc_timer_config_t timer_conf, ledc_channel_config_t channel_conf);

// 设置LCD背光亮度，dc范围10~100
void LCD_Set_Light(uint8_t dc);

#endif
