#ifndef __AT24C02_H
#define __AT24C02_H

#include "stdint.h"

#define AT_ADDRESS	0x50

void AT24C02_Write(uint8_t addr,uint8_t length,uint8_t buff[]);
void AT24C02_Read(uint8_t addr, uint8_t length, uint8_t buff[]);
void AT24C02_Init(void);

#endif
    