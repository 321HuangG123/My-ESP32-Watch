#ifndef __MPU6050_H
#define __MPU6050_H

#include "esp_err.h"

#define MPU6050_ADDR 0x68

#define MPU_SELF_TESTX_REG 0X0D   // 自检寄存器X
#define MPU_SELF_TESTY_REG 0X0E   // 自检寄存器Y
#define MPU_SELF_TESTZ_REG 0X0F   // 自检寄存器Z
#define MPU_SELF_TESTA_REG 0X10   // 自检寄存器A
#define MPU_SAMPLE_RATE_REG 0X19  // 采样频率分频器
#define MPU_CFG_REG 0X1A          // 配置寄存器
#define MPU_GYRO_CFG_REG 0X1B     // 陀螺仪配置寄存器
#define MPU_ACCEL_CFG_REG 0X1C    // 加速度计配置寄存器
#define MPU_MOTION_DET_REG 0X1F   // 运动检测阀值设置寄存器
#define MPU_MOTION_DUR_REG 0X20   // motion time value
#define MPU_FIFO_EN_REG 0X23      // FIFO使能寄存器
#define MPU_I2CMST_CTRL_REG 0X24  // IIC主机控制寄存器
#define MPU_I2CSLV0_ADDR_REG 0X25 // IIC从机0器件地址寄存器
#define MPU_I2CSLV0_REG 0X26      // IIC从机0数据地址寄存器
#define MPU_I2CSLV0_CTRL_REG 0X27 // IIC从机0控制寄存器
#define MPU_I2CSLV1_ADDR_REG 0X28 // IIC从机1器件地址寄存器
#define MPU_I2CSLV1_REG 0X29      // IIC从机1数据地址寄存器
#define MPU_I2CSLV1_CTRL_REG 0X2A // IIC从机1控制寄存器
#define MPU_I2CSLV2_ADDR_REG 0X2B // IIC从机2器件地址寄存器
#define MPU_I2CSLV2_REG 0X2C      // IIC从机2数据地址寄存器
#define MPU_I2CSLV2_CTRL_REG 0X2D // IIC从机2控制寄存器
#define MPU_I2CSLV3_ADDR_REG 0X2E // IIC从机3器件地址寄存器
#define MPU_I2CSLV3_REG 0X2F      // IIC从机3数据地址寄存器
#define MPU_I2CSLV3_CTRL_REG 0X30 // IIC从机3控制寄存器
#define MPU_I2CSLV4_ADDR_REG 0X31 // IIC从机4器件地址寄存器
#define MPU_I2CSLV4_REG 0X32      // IIC从机4数据地址寄存器
#define MPU_I2CSLV4_DO_REG 0X33   // IIC从机4写数据寄存器
#define MPU_I2CSLV4_CTRL_REG 0X34 // IIC从机4控制寄存器
#define MPU_I2CSLV4_DI_REG 0X35   // IIC从机4读数据寄存器

#define MPU_I2CMST_STA_REG 0X36 // IIC主机状态寄存器
#define MPU_INTBP_CFG_REG 0X37  // 中断/旁路设置寄存器
#define MPU_INT_EN_REG 0X38     // 中断使能寄存器
#define MPU_INT_STA_REG 0X3A    // 中断状态寄存器

#define MPU_ACCEL_XOUTH_REG 0X3B // 加速度值,X轴高8位寄存器
#define MPU_ACCEL_XOUTL_REG 0X3C // 加速度值,X轴低8位寄存器
#define MPU_ACCEL_YOUTH_REG 0X3D // 加速度值,Y轴高8位寄存器
#define MPU_ACCEL_YOUTL_REG 0X3E // 加速度值,Y轴低8位寄存器
#define MPU_ACCEL_ZOUTH_REG 0X3F // 加速度值,Z轴高8位寄存器
#define MPU_ACCEL_ZOUTL_REG 0X40 // 加速度值,Z轴低8位寄存器

#define MPU_TEMP_OUTH_REG 0X41 // 温度值高八位寄存器
#define MPU_TEMP_OUTL_REG 0X42 // 温度值低8位寄存器

#define MPU_GYRO_XOUTH_REG 0X43 // 陀螺仪值,X轴高8位寄存器
#define MPU_GYRO_XOUTL_REG 0X44 // 陀螺仪值,X轴低8位寄存器
#define MPU_GYRO_YOUTH_REG 0X45 // 陀螺仪值,Y轴高8位寄存器
#define MPU_GYRO_YOUTL_REG 0X46 // 陀螺仪值,Y轴低8位寄存器
#define MPU_GYRO_ZOUTH_REG 0X47 // 陀螺仪值,Z轴高8位寄存器
#define MPU_GYRO_ZOUTL_REG 0X48 // 陀螺仪值,Z轴低8位寄存器

#define MPU_MOT_DET_STA_REG 0x61 // motion detect status

#define MPU_I2CSLV0_DO_REG 0X63 // IIC从机0数据寄存器
#define MPU_I2CSLV1_DO_REG 0X64 // IIC从机1数据寄存器
#define MPU_I2CSLV2_DO_REG 0X65 // IIC从机2数据寄存器
#define MPU_I2CSLV3_DO_REG 0X66 // IIC从机3数据寄存器

#define MPU_I2CMST_DELAY_REG 0X67 // IIC主机延时管理寄存器
#define MPU_SIGPATH_RST_REG 0X68  // 信号通道复位寄存器
#define MPU_MDETECT_CTRL_REG 0X69 // 运动检测控制寄存器
#define MPU_USER_CTRL_REG 0X6A    // 用户控制寄存器
#define MPU_PWR_MGMT1_REG 0X6B    // 电源管理寄存器1
#define MPU_PWR_MGMT2_REG 0X6C    // 电源管理寄存器2
#define MPU_FIFO_CNTH_REG 0X72    // FIFO计数寄存器高八位
#define MPU_FIFO_CNTL_REG 0X73    // FIFO计数寄存器低八位
#define MPU_FIFO_RW_REG 0X74      // FIFO读写寄存器
#define MPU_WHO_AM_I_REG 0X75     // 器件ID寄存器

/**
 * MPU6050 初始化工作
 * @param   无
 * @return  无
 */
void MPU6050_init(void);

/**
 * 从 MPU6050 读取一个数据
 * @param   RegAddress  寄存器地址
 * @return  寄存器中的数据
 */
uint8_t MPU6050_ReadReg(uint8_t RegAddress);

/**
 * 向MPU6050中写入数据
 * @param   RegAddress      寄存器的地址
 * @param   Data            向寄存器填充的值
 * @return  无
 */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);

/**
 * MPU6050获取ID号
 * @param   无
 * @return  MPU6050 ID号
 */
uint8_t MPU6050_GetID(void);

/**
 * 设置MPU6050陀螺仪传感器满量程范围
 * @param  fsr:0,+250dps;1,500dps;2,+1000dps;3,+2000dps
 * @return 0 if success
 */
uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr);

/**
 * 设置MPU6050的数字低通滤波器
 * @param  fsr:低通滤波器频率(Hz)
 * @return 0 if success
 */
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr);

/**
 * 设置MPU6050的低通滤波器
 * @param  lpf: Hz
 * @return 无
 */
uint8_t MPU6050_Set_LPF(uint16_t lpf);

/**
 * 设置采样率
 * @param   rate    4~1000Hz
 * @return  无
 */
uint8_t MPU6050_Set_Rate(uint16_t rate);

/**
 * 获取MPU6050温度值
 * @param  NULL
 * @return 温度 (short)
 */
short MPU_Get_Temperature(void);

uint8_t MPU6050_Set_Fifo(uint16_t sens);

/**
 * 获取陀螺仪原始值
 * @param   Gyro_   X、Y、Z 轴的原始值
 * @return  无
 */
void MPU6050_GetGyroXData(int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);

/**
 * 获取加速度原始值
 * @param   Acc_    X、Y、Z 轴的原始值
 * @return  无
 */
void MPU6050_GetAccxData(int16_t *AccX, int16_t *AccY, int16_t *AccZ);

/**
 * MPU6050 进入睡眠模式
 * @param   无
 * @return  无
 */
void MPU_Sleep();

/**
 * MPU6050  唤醒
 * @param   无
 * @return  无
 */
void MPU_Wakeup();

/**
 * MPU6050 读取状态位
 * @param   无
 * @return  读取到的中断状态
 */
uint8_t MPU_Read_Status();

/**
 * 利用静态加速度计数据来估算设备的姿态（倾斜角度）
 * @param  roll (float)  绕 X 轴旋转的角度（设备左右倾斜）
 * @param  pitch(float)  绕 Y 轴旋转的角度（设备前后倾斜）
 * @return NULL
 */
void MPU_Get_Angles(float *roll, float *pitch);

/**
 * MPU6050 是否处于水平状态 
 * @param  NULL
 * @return 处于水平 ? 1 : 0
 */
uint8_t MPU_isHorizontal(void);

#endif
