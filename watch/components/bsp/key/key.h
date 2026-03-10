#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>

#define KEY1_PIN       GPIO_NUM_13
#define KEY2_PIN       GPIO_NUM_14
#define KEY1           gpio_get_level(KEY1_PIN)
#define KEY2           gpio_get_level(KEY2_PIN)

void Key_Port_Init(void);
uint8_t KeyScan(uint8_t mode);

#endif
