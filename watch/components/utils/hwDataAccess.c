#include "hwDataAccess.h"
#include "time.h"
#include "adc_power.h"
#include "bmp280/bmp280.h"
#include "dht11/dht11.h"
#include "mpu6050/mpu6050.h"
#include "mpu6050/inv_mpu.c"

/***************************
 *  RTC Fucntions
 ***************************/
/**************************************************************************/
/*!
    @brief  to get the real time clock from the hardware

    @param  nowdatetime to storge the data&time

    @return None
*/
/**************************************************************************/
void HW_RTC_Get_TimeDate(HW_DateTimeTypeDef *nowdatetime)
{

#if HW_USE_RTC
    if (nowdatetime != NULL)
    {
        time_t now;
        struct tm timeinfo;
        // 只是调用 time(&now) 而不联网，也不手动设置，ESP32 启动后默认会从 1970年1月1日 00:00:00 开始计时。

        /**
         * Unix 时间戳是从 1970年1月1日 00:00:00 UTC 到现在的总秒数
         * 而 struct tm 结构体是用来表示本地时间的一个结构体，
         * 并且C语言规定 tm_year 的值代表的是距离 1900 年已经过去的年数
         * 默认情况下，ESP32设置的时间戳是0，（C语言）系统通过计算时间戳会发现当前时间是1970年1月1日 00:00:00(在无设置时区情况下)
         * 所以 tm_year = 1970 - 1900 = 70
         */
        

        time(&now);                                 // 获取当前时间戳
        localtime_r(&now, &timeinfo);               // 获取本地时间
        nowdatetime->Year = timeinfo.tm_year - 100; // 距离1900年的年数，如现在是2026，则tm_year = 126，需要转换成两位数的年份，所以减去100
        nowdatetime->Month = timeinfo.tm_mon + 1;   // tm_mon 范围是 0-11，所以要加1
        nowdatetime->Date = timeinfo.tm_mday;
        nowdatetime->WeekDay = timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday; // tm_wday 范围是 0-6，0代表星期天，这里转换成 1-7，1代表星期一
        nowdatetime->Hours = timeinfo.tm_hour;
        nowdatetime->Minutes = timeinfo.tm_min;
        nowdatetime->Seconds = timeinfo.tm_sec;
    }
#else
    nowdatetime->Year = 26;
    nowdatetime->Month = 1;
    nowdatetime->Date = 1;
    nowdatetime->WeekDay = 4;
    nowdatetime->Hours = 0;
    nowdatetime->Minutes = 0;
    nowdatetime->Seconds = 0;
#endif
}

/**************************************************************************/
/*!
    @brief  日期的设置

    @param  nowdatetime to storge the data&time

	@return None
*/
/**************************************************************************/
void HW_RTC_Set_Date(uint8_t year, uint8_t month, uint8_t date)
{
	#if HW_USE_RTC
        time_t now;
		struct tm timeinfo;
        // 拿到当前时间戳，并转换为结构体
        time(&now);
        localtime_r(&now, &timeinfo);

        // year=26 ==> +2000=2026 ==> -1900=126
        timeinfo.tm_year = (year + 2000) - 1900;  
        timeinfo.tm_mon = month - 1;                            // tm_mon 范围是 0-11，所以要减1
        timeinfo.tm_mday = date;
        
        // 合并写回底层
        time_t t_new = mktime(&timeinfo);                       // 转换为时间戳
        struct timeval tv = { .tv_sec = t_new, .tv_usec = 0 };  // 微秒设为0
        settimeofday(&tv, NULL);                                // 设置系统时间
	#endif
}

/**************************************************************************/
/*!
    @brief  时间的设置

    @param  nowdatetime to storge the data&time

	@return None
*/
/**************************************************************************/
void HW_RTC_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	#if HW_USE_RTC
		time_t now;
		struct tm timeinfo;
        // 拿到当前时间戳，并转换为结构体
        time(&now);
        localtime_r(&now, &timeinfo);

        // year=26 ==> +2000=2026 ==> -1900=126
        timeinfo.tm_hour = hours;  
        timeinfo.tm_min = minutes;              // tm_mon 范围是 0-11，所以要减1
        timeinfo.tm_sec = seconds;
        
        // 合并写回底层
        time_t t_new = mktime(&timeinfo);                       // 转换为时间戳
        struct timeval tv = { .tv_sec = t_new, .tv_usec = 0 };  // 微秒设为0
        settimeofday(&tv, NULL);                // 设置系统时间
	#endif
}

/**************************************************************************/
/*!
    @brief  计算今天是星期几

    @param  nowdatetime to storge the data&time

	@return None

    泽勒公式计算
    // {
    //   	int w;
    //       // 公式规定，如果是1月或者2月，要把月份当作13月或者14月，年份减1
    //   	if (setmonth == 1 || setmonth == 2)
    //   	{setyear--, setmonth += 12;}
    //   	w = setyear + setyear / 4 + century / 4  + 26*(setmonth + 1)/10 + setday - 1 - 2 * century;
    //   	while(w<0)
    //   		w+=7;
    //   	w%=7;
    //   	w=(w==0)?7:w;
    //   	return w;
    // }
*/
/**************************************************************************/
uint8_t HW_weekday_calculate(void)
{
    time_t now;
	struct tm timeinfo;
    // 拿到当前时间戳，并转换为结构体
    time(&now);
    localtime_r(&now, &timeinfo);
    // ESP内部会帮我们计算，不需要泽勒公式
    return timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday;
}

/***************************
 *  Power Fucntions
 ***************************/
/**************************************************************************/
/*!
    @brief initialize the power

    @param	NULL

	@return NULL
*/
/**************************************************************************/
void HW_Power_Init(void)
{
	#if HW_USE_BAT
		// 留空，不做任何操作
	#endif
}

/**************************************************************************/
/*!
    @brief shutdown the power

    @param	NULL

	@return NULL
*/
/**************************************************************************/
void HW_Power_Shutdown(void)
{
	#if HW_USE_BAT
		// 留空，不做任何操作
	#endif
}

/**************************************************************************/
/*!
    @brief	利用ADC计算剩余电量

    @param	NULL

	@return bat power remain
*/
/**************************************************************************/
uint8_t HW_Power_BatCalculate(void)
{
	#if HW_USE_BAT
		return PowerCalculate();
	#endif
		return 0;
}



/***************************
 *  LCD Fucntions
 ***************************/

/**************************************************************************/
/*!
    @brief set the lcd light

    @param	dc the LCD light

	@return NULL
*/
/**************************************************************************/
void HW_LCD_Set_Light(uint8_t dc)
{
	#if HW_USE_LCD
		LCD_Set_Light(dc);
	#endif
}

/***************************
 *  IMU Fucntions
 ***************************/

/**************************************************************************/
/*!
    @brief  initialize the MPU6050

    @param	NULL

	@return 0 if successful
*/
/**************************************************************************/
uint8_t HW_MPU_Init(void)
{
	#if HW_USE_IMU
		return mpu_dmp_init();
	#endif
	return -1;
}


/**************************************************************************/
/*!
    @brief  set the MPU Wrist wake up to enabled

    @param	NULL

	@return NULL
*/
/**************************************************************************/
void HW_MPU_Wrist_Enable(void)
{
	#if HW_USE_IMU
		HWInterface.IMU.wrist_is_enabled = 1;
	#endif
}


/**************************************************************************/
/*!
    @brief  set the MPU Wrist wake up to disabled

    @param	NULL

	@return NULL
*/
/**************************************************************************/
void HW_MPU_Wrist_Disable(void)
{
	#if HW_USE_IMU
		HWInterface.IMU.wrist_is_enabled = 0;
	#endif
}


/**************************************************************************/
/*!
    @brief  get the MPU steps

    @param	NULL

	@return the steps
*/
/**************************************************************************/
uint16_t HW_MPU_Get_Steps(void)
{
	#if HW_USE_IMU
		unsigned long STEPS = 0;
		if(!HWInterface.IMU.ConnectionError)
			dmp_get_pedometer_step_count(&STEPS);
		return (uint16_t)STEPS;
	#endif
		return 0;
}

/**************************************************************************/
/*!
    @brief	set the MPU steps

    @param	NULL

	@return 0 if successful
*/
/**************************************************************************/
int HW_MPU_Set_Steps(unsigned long count)
{
	#if HW_USE_IMU
		if(!HWInterface.IMU.ConnectionError)
			return dmp_set_pedometer_step_count(count);
	#endif
		return -1;
}

/*************************************
 *  temprature & humidity Fucntions
 *************************************/

/**************************************************************************/
/*!
    @brief  initialize the DHT11 - temprature & humidity sensor

    @param	NULL

	@return 0 if successful
*/
/**************************************************************************/
uint8_t HW_DHT11_Init(void)
{
	#if HW_USE_DHT11
		return DHT_Init();
	#endif
	return -1;
}

/**************************************************************************/
/*!
    @brief  get the temperature and humidity data for the dht11 sensor

    @param	humi humidity
	@param	temp temperature

	@return NULL
*/
/**************************************************************************/
void HW_DHT11_Get_Humi_Temp(float *humi, float *temp)
{
	#if HW_USE_DHT11
		//temp and humi messure
		if(!HWInterface.DHT11.ConnectionError)
			DHT_Read(humi,temp);
	#endif
}

/***************************
 *  Barometer Fucntions - BMP280
 ***************************/

/**************************************************************************/
/*!
    @brief  initialize the BMP280 Barometer sensor

    @param	NULL

	@return 0 if successful
*/
/**************************************************************************/
uint8_t HW_Barometer_Init(void)
{
	#if HW_USE_SPL06
		return BMP280_Init();
	#endif

	return -1;
}



/***************************
 *  E-compass Fucntions - LSM303
 ***************************/

/**************************************************************************/
/*!
    @brief  initialize the lsm303 E-compass sensor

    @param	NULL

	@return 0 if successful
*/
/**************************************************************************/
uint8_t HW_Ecompass_Init(void)
{
	#if HW_USE_LSM303
		return LSM303DLH_Init();
	#endif

	return -1;
}

/**************************************************************************/
/*!
    @brief  SET the lsm303 E-compass sensor to sleep mode

    @param	NULL

	@return NULL
*/
/**************************************************************************/
void HW_Ecompass_Sleep(void)
{
	#if HW_USE_LSM303
		LSM303DLH_Sleep();
	#endif
}


/***************************
 *  heart rate meter Fucntions - MAX30102
 ***************************/

/**************************************************************************/
/*!
    @brief  initialize the MAX30102 heart rate sensor

    @param	NULL

	@return 0 if successful
*/

/**************************************************************************/
uint8_t HW_HRmeter_Init(void)
{
	#if HW_USE_MAX30102
		return Init_MAX30102();
	#endif

	return -1;

}

/**************************************************************************/
/*!
    @brief  SET the MAX30102 HR sensor to sleep mode

    @param	NULL

	@return NULL
*/
/**************************************************************************/
void HW_HRmeter_Sleep(void)
{
	#if HW_USE_MAX30102
		MAX30102_hrs_DisEnable();
	#endif
}


/***************************
 *  External Variables
 ***************************/
HW_InterfaceTypeDef HWInterface = {
    .RealTimeClock = {
        .GetTimeDate = HW_RTC_Get_TimeDate,
        .SetDate = HW_RTC_Set_Date,
        .SetTime = HW_RTC_Set_Time,
        .CalculateWeekday = HW_weekday_calculate
    },
    // .BLE = {
	// 	.Init = HW_BLE_Init,
    //     .Enable = HW_BLE_Enable,
    //     .Disable = HW_BLE_Disable
    // },
    .Power = {
		.power_remain = 0,
		.Init = HW_Power_Init,
        .Shutdown = HW_Power_Shutdown,
		.BatCalculate = HW_Power_BatCalculate
    },
    .LCD = {
        .SetLight = HW_LCD_Set_Light
    },
	.IMU = {
		.ConnectionError = 1,
		.Steps = 0,
		.wrist_is_enabled = 0,
		.wrist_state = WRIST_UP,
		.Init = HW_MPU_Init,    
        .WristEnable = HW_MPU_Wrist_Enable,
        .WristDisable = HW_MPU_Wrist_Disable,
        .GetSteps = HW_MPU_Get_Steps,
		.SetSteps = HW_MPU_Set_Steps
    },
	.DHT11 = {
		.ConnectionError = 1,
		.humidity = 67,
		.temperature = 26,
		.Init = HW_DHT11_Init,
		.GetHumiTemp = HW_DHT11_Get_Humi_Temp
	},
	.Barometer = {
		.ConnectionError = 1,
		.altitude = 19,
		.Init = HW_Barometer_Init,
	},
	.Ecompass = {
		.ConnectionError = 1,
		.direction = 45,
		.Init = HW_Ecompass_Init,
		.Sleep = HW_Ecompass_Sleep
	},
	.HR_meter = {
		.ConnectionError = 1,
		.HrRate = 0,
		.SPO2 = 99,
		.Init = HW_HRmeter_Init,
		.Sleep = HW_HRmeter_Sleep
	},

    // 添加wifi
};