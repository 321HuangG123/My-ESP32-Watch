#include "spl06_001.h"
#include "driver/i2c.h"
#include "math.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 定义结构体
typedef struct {
    i2c_port_t port;      // I2C 端口号 (I2C_NUM_0 或 I2C_NUM_1)
    uint32_t scl_io;      // SCL 引脚编号
    uint32_t sda_io;      // SDA 引脚编号
} esp32_i2c_bus_t;

// 写入一个字节
uint8_t IIC_Write_One_Byte(esp32_i2c_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    esp_err_t err = i2c_master_write_to_device(bus->port, daddr, write_buf, 2, pdMS_TO_TICKS(100));
    return (err == ESP_OK) ? 0 : 1;
}

// 读取一个字节
unsigned char IIC_Read_One_Byte(esp32_i2c_bus_t *bus, uint8_t daddr, uint8_t reg) {
    uint8_t data = 0;
    // 注意：daddr 传入 0x76 即可，无需左移
    esp_err_t err = i2c_master_write_read_device(bus->port, daddr, &reg, 1, &data, 1, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        return 0; // 或者返回一个错误码
    }
    return data;
}

// 初始化硬件 I2C
void IICInit(esp32_i2c_bus_t *bus) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = bus->sda_io,
        .scl_io_num = bus->scl_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000, // 400KHz
    };
    i2c_param_config(bus->port, &conf);
    i2c_driver_install(bus->port, conf.mode, 0, 0, 0);
}

// 1. 定义 ESP32 上的引脚连接
esp32_i2c_bus_t bus = {
    .port = I2C_NUM_0,
    .sda_io = SPL06_001_SDA_PIN,  
    .scl_io = SPL06_001_SCL_PIN,  
};

int16_t c0,c1,c01,c11,c20,c21,c30;
int32_t c00,c10;

uint8_t SPL_ReadOneReg(uint8_t addr)
{
	uint8_t dat;
	dat = IIC_Read_One_Byte(&bus, SPL_CHIP_ADDRESS,addr);
	return dat;
}

void SPL_WriteOneReg(uint8_t addr, uint8_t dat)
{
	IIC_Write_One_Byte(&bus, SPL_CHIP_ADDRESS,addr,dat);
}

int32_t Get_Traw()
{
	uint8_t buff[3];
	int32_t Traw;
	buff[0] = SPL_ReadOneReg(SPL_TMP_B0);
	buff[1] = SPL_ReadOneReg(SPL_TMP_B1);
	buff[2] = SPL_ReadOneReg(SPL_TMP_B2);
	Traw = buff[2];
	Traw = Traw << 8 | buff[1];
	Traw = Traw << 8 | buff[0];
	if(Traw & (1<<23))
	{Traw |= 0xFF000000;}
	return Traw;
}

int32_t Get_Praw()
{
	uint8_t buff[3];
	int32_t Praw;
	buff[0] = SPL_ReadOneReg(SPL_PRS_B0);
	buff[1] = SPL_ReadOneReg(SPL_PRS_B1);
	buff[2] = SPL_ReadOneReg(SPL_PRS_B2);
	Praw = buff[2];
	Praw = Praw << 8 | buff[1];
	Praw = Praw << 8 | buff[0];
	if(Praw & (1<<23))
	{Praw |= 0xFF000000;}
	return Praw;
}

int16_t get_c0()
{
	uint8_t buff[2];
	int16_t c0;
	buff[0] = SPL_ReadOneReg(COEF_C0);
	buff[1] = SPL_ReadOneReg(COEF_C0_C1);
	c0 = buff[0];
	c0 = (c0 << 4) | (buff[1] >> 4);
	if(c0 & (1<<11))
	{c0 |= 0xF000;}
	return c0;
}

int16_t get_c1()
{
	uint8_t buff[2];
	int16_t c1;
	buff[0] = SPL_ReadOneReg(COEF_C0_C1);
	buff[1] = SPL_ReadOneReg(COEF_C1);
	c1 = buff[0] & 0x0F;
	c1 = (c1 << 8) | buff[1] ;
	if(c1 & (1<<11))
	{c1 |= 0xF000;}
	return c1;
}

int32_t get_c00()
{
	uint8_t buff[3];
	int32_t c00;
	buff[0] = SPL_ReadOneReg(COEF_C00_H);
	buff[1] = SPL_ReadOneReg(COEF_C00_L);
	buff[2] = SPL_ReadOneReg(COEF_C00_C10);
	c00 = buff[0];
	c00 = c00<<8 | buff[1];
	c00 = (c00<<4) | (buff[2]>>4);
	if(c00 & (1<<19))
	{c00 |= 0xFFF00000;}
	return c00;
}

int32_t get_c10()
{
	uint8_t buff[3];
	int32_t c10;
	buff[0] = SPL_ReadOneReg(COEF_C00_C10);
	buff[1] = SPL_ReadOneReg(COEF_C10_M);
	buff[2] = SPL_ReadOneReg(COEF_C10_L);
	c10 = buff[0] & 0x0F;
	c10 = c10<<8 | buff[1];
	c10 = c10<<8 | buff[2];
	if(c10 & (1<<19))
	{c10 |= 0xFFF00000;}
	return c10;
}

int16_t get_c01()
{
	uint8_t buff[2];
	int32_t c01;
	buff[0] = SPL_ReadOneReg(COEF_C01_H);
	buff[1] = SPL_ReadOneReg(COEF_C01_L);
	c01 = buff[0];
	c01 = c01<<8 | buff[1];
	return c01;
}

int16_t get_c11()
{
	uint8_t buff[2];
	int32_t c11;
	buff[0] = SPL_ReadOneReg(COEF_C11_H);
	buff[1] = SPL_ReadOneReg(COEF_C11_L);
	c11 = buff[0];
	c11 = c11<<8 | buff[1];
	return c11;
}

int16_t get_c20()
{
	uint8_t buff[2];
	int32_t c20;
	buff[0] = SPL_ReadOneReg(COEF_C20_H);
	buff[1] = SPL_ReadOneReg(COEF_C20_L);
	c20 = buff[0];
	c20 = c20<<8 | buff[1];
	return c20;
}

int16_t get_c21()
{
	uint8_t buff[2];
	int32_t c21;
	buff[0] = SPL_ReadOneReg(COEF_C21_H);
	buff[1] = SPL_ReadOneReg(COEF_C21_L);
	c21 = buff[0];
	c21 = c21<<8 | buff[1];
	return c21;
}

int16_t get_c30()
{
	uint8_t buff[2];
	int32_t c30;
	buff[0] = SPL_ReadOneReg(COEF_C30_H);
	buff[1] = SPL_ReadOneReg(COEF_C30_L);
	c30 = buff[0];
	c30 = c30<<8 | buff[1];
	return c30;
}

uint8_t SPL_init()
{
	IICInit(&bus);
	
	SPL_WriteOneReg(SPL_PRS_CFG, 0x01);		// Pressure 2x oversampling

	SPL_WriteOneReg(SPL_TMP_CFG, 0x80);		// External Temperature 1x oversampling

	SPL_WriteOneReg(SPL_MEAS_CFG, 0x07);	// continuous pressure and temperature measurement

	SPL_WriteOneReg(SPL_CFG_REG, 0x00);		//   
	
	c0 = get_c0();
	c1 = get_c1();
	c01 = get_c01();
	c11 = get_c11(); 
	c20 = get_c20();
	c21 = get_c21();
	c30 = get_c30();
	c00 = get_c00();
	c10 = get_c10();
	
	if(SPL_ReadOneReg(SPL_PRS_CFG)!=0x01 || SPL_ReadOneReg(SPL_CFG_REG)!=0x00)
	{return 1;}//ERRO
	else 
	{return 0;}//SUCCESS
}

void SPL_Sleep()
{
	SPL_WriteOneReg(SPL_MEAS_CFG, 0x00);	// standby
	//SPL_WriteOneReg(SPL_RESET_REG,0x09);//reset
}
	
void SPL_Wakeup()
{
	SPL_WriteOneReg(SPL_MEAS_CFG, 0x07);	// continuous pressure and temperature measurement
	//SPL_init();
}

uint8_t SPL_GetID()
{
	return SPL_ReadOneReg(SPL_ID_REG);
}

// 摄氏度
float Temperature_Calculate()
{
	float Traw_sc, Tcomp;
	Traw_sc = Get_Traw();
	Traw_sc /= KT;
	Tcomp = c0 * 0.5 + c1 * Traw_sc;
	return Tcomp;
}

// 大气压力
float Pressure_Calculate()
{
	float Traw_sc, Praw_sc, Pcomp;
	Traw_sc = Get_Traw();
	Traw_sc /= KT;
	Praw_sc = Get_Praw();
	Praw_sc /= KP;
	Pcomp = (c00) + Praw_sc * ((c10) + Praw_sc * ((c20) + Praw_sc * (c30))) + Traw_sc * (c01) + Traw_sc * Praw_sc * ((c11) + Praw_sc * (c21));
	 
	return Pcomp;
}

// 海拔高度
float Altitude_Calculate()
{
	float Altitude;
	Altitude = 44330 * (1 - powf(Pressure_Calculate()/101325, 0.1903));
	return Altitude;
}
