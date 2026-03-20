#ifndef __HWDATAACCESS_H__
#define __HWDATAACCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/***************************
 *  Hardware Define
 ***************************/
/**
 *  如果不使用硬件, 就设置为 0
 *
 *
 *  if just test ui, no hardware, just set HW_USE_HARDWARE 0
 *
 */

#define HW_USE_HARDWARE 1

#if HW_USE_HARDWARE
  #define HW_USE_RTC      1       // RTC实时时钟
  #define HW_USE_BLE      0       // 蓝牙模块
  #define HW_USE_BAT      1       // 电池管理模块
  #define HW_USE_LCD      1       // LCD显示屏模块
  #define HW_USE_IMU      1       // 六轴传感器模块
  #define HW_USE_DHT11    1       // 温湿度传感器模块
  #define HW_USE_SPL06    1       // 气压传感器模块
  #define HW_USE_LSM303   1       // 电子罗盘模块
  #define HW_USE_MAX30102 1       // 心率血氧模块
#endif

#if HW_USE_BLE
  #include "kt6328.h"
#endif

#if HW_USE_BAT
  // #include "power.h"
#endif

#if HW_USE_LCD
  #include "screen/lcd_init.h"
#endif

#if HW_USE_IMU
  #include "mpu6050/mpu6050.h"
  #include "mpu6050/inv_mpu.h"
  #include "mpu6050/inv_mpu_dmp_motion_driver.h"
#endif

#if HW_USE_DHT11
  #include "dht11/dht11.h"
#endif

#if HW_USE_SPL06
  #include "spl06_001/spl06_001.h"
#endif

#if HW_USE_LSM303
  #include "lsm303/lsm303.h"
#endif

#if HW_USE_MAX30102
  #include "max30102/max30102_read.h"
#endif


/***************************
 *  TypeDefs
 ***************************/
/**
  * @brief  HW RTC DateTime structure definition
  */
typedef struct
{
    uint8_t WeekDay;    /*!< Specifies the RTC Date WeekDay.
                            This parameter can be a value of @ref RTC_WeekDay_Definitions */

    uint8_t Month;      /*!< Specifies the RTC Date Month (in BCD format).
                            This parameter can be a value of @ref RTC_Month_Date_Definitions */

    uint8_t Date;       /*!< Specifies the RTC Date.
                            This parameter must be a number between Min_Data = 1 and Max_Data = 31 */

    uint8_t Year;       /*!< Specifies the RTC Date Year.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 99 */

    uint8_t Hours;      /*!< Specifies the RTC Time Hour.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 12 if the RTC_HourFormat_12 is selected
                            This parameter must be a number between Min_Data = 0 and Max_Data = 23 if the RTC_HourFormat_24 is selected */

    uint8_t Minutes;    /*!< Specifies the RTC Time Minutes.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

    uint8_t Seconds;    /*!< Specifies the RTC Time Seconds.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

} HW_DateTimeTypeDef;


/**
  * @brief  HW RTC Interface definition
  */
typedef struct
{
    void (*GetTimeDate)(HW_DateTimeTypeDef *nowdatetime);
    void (*SetDate)(uint8_t year, uint8_t month, uint8_t date);
    void (*SetTime)(uint8_t hours, uint8_t minutes, uint8_t seconds);
    uint8_t (*CalculateWeekday)(uint8_t setyear, uint8_t setmonth, uint8_t setday, uint8_t century);
} HW_RTC_InterfaceTypeDef;

/**
  * @brief  HW BLE Interface definition
  */
typedef struct
{
    void (*Init)(void);
    void (*Enable)(void);
    void (*Disable)(void);
} HW_BLE_InterfaceTypeDef;


/**
  * @brief  HW Power Interface definition
  */
typedef struct
{
    uint8_t power_remain;

    void (*Init)(void);
    void (*Shutdown)(void);
    uint8_t (*BatCalculate)(void);
} HW_Power_InterfaceTypeDef;

/**
  * @brief  HW LCD Interface definition
  */
typedef struct
{
    void (*SetLight)(uint8_t dc);
} HW_LCD_InterfaceTypeDef;

/**
  * @brief  HW IMU wrist state defines
  * 手腕状态定义，抬起和放下
  */
#define WRIST_UP 1
#define WRIST_DOWN 0

/**
  * @brief  HW IMU Interface definition
  */
typedef struct
{
    uint8_t ConnectionError;
    uint16_t Steps;
    uint8_t wrist_state;
    uint8_t wrist_is_enabled;

    uint8_t (*Init)(void);
    void (*WristEnable)(void);
    void (*WristDisable)(void);
    uint16_t (*GetSteps)(void);
    int (*SetSteps)(unsigned long count);
} HW_IMU_InterfaceTypeDef;

/**
  * @brief  HW DHT11 Interface definition
  */
typedef struct
{
  uint8_t ConnectionError;
  uint8_t temperature;
  uint8_t humidity;
  uint8_t (*Init)(void);
  void (*GetHumiTemp)(float *humi, float *temp);
} HW_DHT11_InterfaceTypeDef;


/**
  * @brief  HW SPL06-001 Barometer Interface definition
  */
typedef struct
{
  uint8_t ConnectionError;
  uint16_t altitude;
  uint8_t (*Init)(void);

} HW_Barometer_InterfaceTypeDef;

/**
  * @brief  HW LSM_303 E-compass Interface definition
  */
typedef struct
{
  uint8_t ConnectionError;
  uint16_t direction;
  uint8_t (*Init)(void);
  void (*Sleep)(void);

} HW_Ecompass_InterfaceTypeDef;

/**
  * @brief  HW EM7028 heart rate meter Interface definition
  */
typedef struct
{
  uint8_t ConnectionError;
  uint8_t HrRate;
  uint8_t SPO2;
  uint8_t (*Init)(void);
  void (*Sleep)(void);

} HW_HRmeter_InterfaceTypeDef;

/**
  * @brief  Hardware Interface structure definition
  */
typedef struct
{
    HW_RTC_InterfaceTypeDef RealTimeClock;
    HW_BLE_InterfaceTypeDef BLE;
    HW_Power_InterfaceTypeDef Power;
    HW_LCD_InterfaceTypeDef LCD;
    HW_IMU_InterfaceTypeDef IMU;
    HW_DHT11_InterfaceTypeDef DHT11;
    HW_Barometer_InterfaceTypeDef Barometer;
    HW_Ecompass_InterfaceTypeDef Ecompass;
    HW_HRmeter_InterfaceTypeDef HR_meter;
} HW_InterfaceTypeDef;


/***************************
 *  External Variables
 ***************************/
extern HW_InterfaceTypeDef HWInterface;

#ifdef __cplusplus
}
#endif

#endif
