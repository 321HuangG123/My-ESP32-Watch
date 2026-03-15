#ifndef _MAX30102_READ_H_
#define _MAX30102_READ_H_

#include <stdint.h>

extern int8_t ch_spo2_valid;      
extern int8_t ch_hr_valid;      

uint8_t Init_MAX30102(void);
void ReadHeartRateSpO2(void);
void bsp_InitI2C(void);
uint8_t i2c_CheckDevice(uint8_t _Address);

#endif 

