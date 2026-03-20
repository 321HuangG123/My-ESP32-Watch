#include "lcd_init.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#include <string.h>


// 全局 SPI 句柄
static spi_device_handle_t spi_handle;

/******************************************************************************
      函数说明：LCD端口初始化
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_GPIO_Init(void)
{
	// 1. 配置 GPIO (RES, DC, CS, BLK)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<RES_PIN) | (1ULL<<DC_PIN) | (1ULL<<BLK_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = true
    };
    gpio_config(&io_conf);

    // 2. 配置 SPI 总线
    spi_bus_config_t buscfg = {
        .miso_io_num = -1, // ST7789 通常不需要读
        .mosi_io_num = SDA_PIN,
        .sclk_io_num = SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 240 * 2 // 最大传输字节
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // 3. 添加设备到总线
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000, // 40MHz
        .mode = 0,                          // SPI Mode 0
        .spics_io_num = CS_PIN,         // 硬件 CS 引脚
        .queue_size = 7,
    };
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
}


/******************************************************************************
      函数说明：LCD串行数据写入函数(software SPI)
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;            // 传输 8 bit
    t.tx_buffer = &dat;
    spi_device_polling_transmit(spi_handle, &t); // 同步传输
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
	gpio_set_level(DC_PIN, 1); // 高电平：数据
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{
	gpio_set_level(DC_PIN, 1);
    LCD_Writ_Bus(dat >> 8);   // 先发高 8 位
    LCD_Writ_Bus(dat & 0xFF); // 后发低 8 位
	
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{
	gpio_set_level(DC_PIN, 0); // 低电平：命令
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	LCD_WR_REG(0x2a);//列地址设置
	LCD_WR_DATA(x1);
	LCD_WR_DATA(x2);
	LCD_WR_REG(0x2b);//行地址设置
	LCD_WR_DATA(y1);
	LCD_WR_DATA(y2);
	LCD_WR_REG(0x2c);//储存器写
}


/******************************************************************************
      函数说明：LCD调节背光
      入口数据：dc,占空比,5%~100%
      返回值：  无
******************************************************************************/
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


/******************************************************************************
      函数说明：LCD关闭背光
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Close_Light(void)
{
    // 计算占空比
    uint32_t duty = (8191 * 0) / 100;  // 13位分辨率，最大值为8191

    // 设置新的占空比
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}


/******************************************************************************
      函数说明：LCD开启背光
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Open_Light(void)
{
	// 计算占空比
    uint32_t duty = (8191 * 100) / 100;  // 13位分辨率，最大值为8191

    // 设置新的占空比
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

/******************************************************************************
      函数说明：ST7789 SLEEP IN
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_ST7789_SleepIn(void)
{
	LCD_WR_REG(0x10);
	vTaskDelay(pdMS_TO_TICKS(100));
}


/******************************************************************************
      函数说明：ST7789 SLEEP OUT
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_ST7789_SleepOut(void)
{
	LCD_WR_REG(0x11);
	vTaskDelay(pdMS_TO_TICKS(100));
}


/******************************************************************************
      函数说明：LCD初始化
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO
	LCD_CS_Clr();		//chip select
	
	LCD_RES_Clr();	//复位
	vTaskDelay(pdMS_TO_TICKS(100));
	LCD_RES_Set();
	vTaskDelay(pdMS_TO_TICKS(100));
	
	LCD_WR_REG(0x11); 
	vTaskDelay(pdMS_TO_TICKS(120));
	LCD_WR_REG(0x36); 
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33); 

	LCD_WR_REG(0xB7); 
	LCD_WR_DATA8(0x35);  

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x12);   

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20);  

	LCD_WR_REG(0xC6); 
	LCD_WR_DATA8(0x0F);    

	LCD_WR_REG(0xD0); 
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0x21); 

	LCD_WR_REG(0x29); 
}

void LCD_WR_DATA_BLOCK(uint8_t *data, uint32_t len)
{
    if (len == 0) return;
    
    gpio_set_level(DC_PIN, 1); // 数据模式
    
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;      // 长度单位是 bit
    t.tx_buffer = data;      // 指向数据缓冲区
    
    spi_device_polling_transmit(spi_handle, &t); // 使用轮询方式发送大块数据
}

