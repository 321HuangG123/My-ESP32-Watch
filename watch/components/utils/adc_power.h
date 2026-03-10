#ifndef __ADC_POWER_H__
#define __ADC_POWER_H__

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define BAT_ADC_CHAN          ADC_CHANNEL_0 

void Power_ADC_Init(adc_channel_t channel);

uint8_t PowerCalculate(void);

#endif
