/* Private includes -----------------------------------------------------------*/
// includes
#include "user_taskInit.h"
#include "user_ScrRenewTask.h"
#include "user_SensUpdateTask.h"
#include "Screens/ui_HomePage.h"
#include "Screens/ui_MenuPage.h"
#include "Screens/ui_SetPage.h"
#include "Screens/ui_HRPage.h"
#include "Screens/ui_SPO2Page.h"
#include "Screens/ui_ENVPage.h"
#include "Screens/ui_CompassPage.h"

#include "dht11/dht11.h"
#include "lsm303/LSM303.h"
#include "spl06_001/SPL06_001.h"
#include "max30102/max30102.h"
#include "max30102/max30102_read.h"
#include "mpu6050/mpu6050.h"

#include "hwDataAccess.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint32_t user_HR_timecount = 0;

/* Private function prototypes -----------------------------------------------*/
// 这是EM7028官方lib的库函数, 没有lib用不了
extern uint8_t GET_BP_MAX(void);
extern uint8_t GET_BP_MIN(void);
extern void Blood_Process(void);
extern void Blood_50ms_process(void);
extern void Blood_500ms_process(void);
extern int em70xx_bpm_dynamic(int RECEIVED_BYTE, int g_sensor_x, int g_sensor_y, int g_sensor_z);
extern int em70xx_reset(int ref);

/**
 * @brief  MPU6050 Check the state
 * @param  argument: Not used
 * @retval None
 */
void MPUCheckTask(void *argument)
{
	while (1)
	{
		if (HWInterface.IMU.wrist_is_enabled)	// 开启了抬腕亮屏
		{
			if (MPU_isHorizontal())		// 处于水平状态
			{
				HWInterface.IMU.wrist_state = WRIST_UP;	// 手腕抬起
			}
			else
			{	// 不处于水平状态
				if (HWInterface.IMU.wrist_state == WRIST_UP)
				{	// 
					HWInterface.IMU.wrist_state = WRIST_DOWN;
					if (Page_Get_NowPage()->page_obj == &ui_HomePage ||
						Page_Get_NowPage()->page_obj == &ui_MenuPage ||
						Page_Get_NowPage()->page_obj == &ui_SetPage)		// 非测量页面
					{
						uint8_t Stopstr;
						xQueueSendToBack(Stop_MessageQueue, &Stopstr, 1); // 进入低功耗模式
					}
					// 若是特定检测页面则继续运行
				}
				// 上面直接进入睡眠模式了，这里单独处理
				HWInterface.IMU.wrist_state = WRIST_DOWN;
			}
		}
		vTaskDelay(pdMS_TO_TICKS(300));
	}
}

/**
 * @brief  HWInterface结构体心率测量数据更新任务
 * @param  argument: Not used
 * @retval None
 */
void HRDataUpdateTask(void *argument)
{
	uint8_t IdleBreakstr = 0;
	// uint16_t dat = 0;
	uint8_t hr_temp = 0;
	while (1)
	{
		if (Page_Get_NowPage()->page_obj == &ui_HRPage)		// 当前处于心率检测页面
		{
			xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 1);		// 解除空闲状态，正常工作模式
			
			// sensor wake up
			maxim_max30102_reset();	// 唤醒传感器
			// receive the sensor wakeup message, sensor wakeup
			if (!HWInterface.HR_meter.ConnectionError)
			{
				// Hr messure
				vTaskSuspendAll();		// 停止调度器，以免测量时任务被切换，结果不准确
				ReadHeartRateSpO2();		// 算法读取心率数据，更新到全局变量
				hr_temp = ch_hr_valid;		
				xTaskResumeAll();		// 恢复调度器
				if (HWInterface.HR_meter.HrRate != hr_temp && hr_temp > 40 && hr_temp < 250)
				{	// 若心率在正常范围内
					HWInterface.HR_meter.HrRate = hr_temp;		// 更新结构体值
				}
			}
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

/**
 * @brief  HWInterface结构体里传感器数据更新
 * @param  argument: Not used
 * @retval None
 */
void SensorDataUpdateTask(void *argument)
{
	// uint8_t value_strbuf[6];
	uint8_t IdleBreakstr = 0;
	while (1)
	{
		// 主页面内容的更新
		uint8_t HomeUpdataStr;
		if (xQueueReceive(HomeUpdata_MessageQueue, &HomeUpdataStr, 0) == pdPASS)// 主页面更新队列有内容
		{	
			// bat
			// uint8_t value_strbuf[5];

			HWInterface.Power.power_remain = HWInterface.Power.BatCalculate();		// 剩余电量
			if (HWInterface.Power.power_remain > 0 && HWInterface.Power.power_remain <= 100)
			{
			}
			else
			{
				HWInterface.Power.power_remain = 0;
			}

			// steps
			if (!(HWInterface.IMU.ConnectionError))
			{	// 获取MPU DMP中的步数
				HWInterface.IMU.Steps = HWInterface.IMU.GetSteps();
			}

			// temp and humi
			if (!(HWInterface.DHT11.ConnectionError))
			{
				// temp and humi messure
				float humi, temp;
				HWInterface.DHT11.GetHumiTemp(&humi, &temp);			// 获取温湿度
				if (temp > -10 && temp < 50 && humi > 0 && humi < 100)
				{	// 检查温湿度是否在合理范围内
					HWInterface.DHT11.humidity = humi;
					HWInterface.DHT11.temperature = temp;
				}
			}

			// send data save message queue
			uint8_t Datastr = 3;
			xQueueSendToBack(DataSave_MessageQueue, &Datastr, 1);	// 通过消息队列告知需要保存
		}

	
		if (Page_Get_NowPage()->page_obj == &ui_SPO2Page)		// 若位于心率测量页面
		{
			xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 1);	// 解除空闲状态，心率测量模块解除睡眠模式进入工作模式
			// receive the sensor wakeup message, sensor wakeup
			if (0)
			{
				// SPO2 messure		// 这里上面有HRDataUpdateTask 专门做心率更新的任务，所以原作者这里不做处理
			}
		}
		
		else if (Page_Get_NowPage()->page_obj == &ui_EnvPage)	// 环境监测页面（温湿度）
		{
			xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 1);		// 解除空闲状态，正常工作模式
			// receive the sensor wakeup message, sensor wakeup
			if (!HWInterface.DHT11.ConnectionError)
			{
				// temp and humi messure
				float humi, temp;
				HWInterface.DHT11.GetHumiTemp(&humi, &temp);		// 获取当前温湿度
				// check
				if (temp > -10 && temp < 50 && humi > 0 && humi < 100)
				{	// 数值范围检测
					HWInterface.DHT11.temperature = (int8_t)temp;
					HWInterface.DHT11.humidity = (int8_t)humi;
				}
			}
		}
		
		else if (Page_Get_NowPage()->page_obj == &ui_CompassPage)	// 电子罗盘（指南针）
		{
			xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 1);
			
			LSM303DLH_Wakeup();			// 唤醒电子罗盘传感器

			// if the sensor is no problem
			if (!HWInterface.Ecompass.ConnectionError)
			{
				// messure
				int16_t Xa, Ya, Za, Xm, Ym, Zm;			
				LSM303_ReadAcceleration(&Xa, &Ya, &Za);
				LSM303_ReadMagnetic(&Xm, &Ym, &Zm);

				// 倾斜补偿方位角计算 计算得指针角度temp
				float temp = Azimuth_Calculate(Xa, Ya, Za, Xm, Ym, Zm) + 0; // 0 offset
				if (temp < 0)
				{
					temp += 360;
				}
				// check
				if (temp >= 0 && temp <= 360)
				{
					HWInterface.Ecompass.direction = (uint16_t)temp;
				}
			}
			// if the sensor is no problem
			if (!HWInterface.Barometer.ConnectionError)		
			{
				// messure
				float alti = Altitude_Calculate();	// 海拔高度
				// check
				if (1)
				{
					HWInterface.Barometer.altitude = (int16_t)alti;
				}
			}
		}

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
