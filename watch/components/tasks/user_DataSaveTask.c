/* Private includes -----------------------------------------------------------*/
// includes
#include "user_DataSaveTask.h"
// APP SYS setting
#include "Screens/ui_DateTimeSetPage.h"
#include "Screens/ui_HomePage.h"

#include "at24c02/DataSave.h"
#include "mpu6050/inv_mpu_dmp_motion_driver.h"

#include "hwDataAccess.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "time.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/******************************************
EEPROM Data description:
[0x00]:0x55 for check
[0x01]:0xAA for check

[0x10]:user wrist setting, HWInterface.IMU.wrist_is_enabled
[0x11]:user ui_APPSy_EN setting

[0x20]:Last Save Day(0-31)
[0x21]:Day Steps

*******************************************/

/* Private function prototypes -----------------------------------------------*/

/* Tasks ---------------------------------------------------------------------*/

/**
 * @brief  Data Save in the EEPROM
 * @param  argument: Not used
 * @retval None
 */
void DataSaveTask(void *argument)
{

	while (1)
	{
		uint8_t Datastr = 0;
		if (xQueueReceive(DataSave_MessageQueue, &Datastr, 1) == pdPASS) // 当队列中有消息表明需要进行数据存储更新
		{
			/****************
			Setting change
			date change
			Step change
			****************/
			uint8_t dat[3];
			dat[0] = HWInterface.IMU.wrist_is_enabled;
			dat[1] = ui_APPSy_EN;
			SettingSave(dat, 0x10, 2);

			// 1. 获取系统当前时间
			time_t now;
			struct tm timeinfo;
			time(&now);
			localtime_r(&now, &timeinfo);

			// timeinfo.tm_mday 范围是 1-31，对应 STM32 的 nowdate.Date
			uint8_t current_day = (uint8_t)timeinfo.tm_mday;
			
			// 2. 从存储中读取旧数据（假设 SettingGet 接口保持不变）
			SettingGet(dat, 0x20, 3);
			
			// 3. 日期比对逻辑
			if (dat[0] != current_day)
			{
				if (!HWInterface.IMU.ConnectionError)
					HWInterface.IMU.SetSteps(0); // 如果日期变了，步数重置为0

				dat[0] = current_day;
				dat[2] = 0;
				dat[1] = 0;
				SettingSave(dat, 0x20, 3);
			}
			else
			{
				uint16_t temp = HWInterface.IMU.GetSteps();
				dat[0] = current_day;
				dat[2] = temp & 0xff;
				dat[1] = temp >> 8 & 0xff;
				SettingSave(dat, 0x20, 3);
			}

		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
