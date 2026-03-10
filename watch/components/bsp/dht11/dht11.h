#ifndef __DHT11_H__
#define __DHT11_H__

#include "driver/gpio.h"

#define DHT11_PIN       GPIO_NUM_2                // DHT11数据引脚连接到GPIO4

uint8_t DHT_Read_Status(void);
void DHT_Reset(void);
uint8_t DHT_Init(void);
uint8_t DHT_Read(int *humi, int *temp);

#endif
