#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "stdint.h"
#include "driver/gpio.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 280

#else
#define LCD_W 280
#define LCD_H 240
#endif


//-----------------LCD端口定义---------------- 
#define SCLK_PIN			3

#define SDA_PIN				5

#define RES_PIN				7

#define DC_PIN				9

#define CS_PIN				8

#define BLK_PIN				0

#define LCD_SCLK_Clr() gpio_set_level(SCLK_PIN, 0)//SCL=SCLK
#define LCD_SCLK_Set() gpio_set_level(SCLK_PIN, 1)

#define LCD_MOSI_Clr() gpio_set_level(SDA_PIN, 0)//SDA=MOSI
#define LCD_MOSI_Set() gpio_set_level(SDA_PIN, 1)

#define LCD_RES_Clr()  gpio_set_level(RES_PIN, 0)//RES
#define LCD_RES_Set()  gpio_set_level(RES_PIN, 1)

#define LCD_DC_Clr()   gpio_set_level(DC_PIN, 0)//DC
#define LCD_DC_Set()   gpio_set_level(DC_PIN, 1)
 		     
#define LCD_CS_Clr()   gpio_set_level(CS_PIN, 0)//CS
#define LCD_CS_Set()   gpio_set_level(CS_PIN, 1)

#define LCD_BLK_Clr()  gpio_set_level(BLK_PIN, 0)//BLK
#define LCD_BLK_Set()  gpio_set_level(BLK_PIN, 1)

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
void LCD_Set_Light(uint8_t dc);
void LCD_Close_Light(void);
void LCD_ST7789_SleepIn(void);
void LCD_ST7789_SleepOut(void);
void LCD_Open_Light(void);
void LCD_WR_DATA_BLOCK(uint8_t *data, uint32_t len);
#endif




