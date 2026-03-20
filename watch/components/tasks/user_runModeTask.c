/* Private includes -----------------------------------------------------------*/
// includes
#include "user_taskInit.h"
#include "ui.h"
#include "Screens/ui_HomePage.h"
#include "Screens/ui_OffTimePage.h"
#include "screen/lcd_init.h"
#include "adc_power.h"
#include "screen/CST816.h"
#include "mpu6050/mpu6050.h"
#include "key/key.h"
#include "hwDataAccess.h"
#include "pwm.h"
#include "esp_task_wdt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_sleep.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t IdleTimerCount = 0;
uint8_t HardInt_Charg_flag = 0;		// 充电标志位，0：未充电；1：正在充电 由于没有电源模块，我们这里就直接让它等于0就行了

/* Private function prototypes -----------------------------------------------*/

/* Tasks ---------------------------------------------------------------------*/

/**
 * @brief  Enter Idle state
 * @param  argument: Not used
 * @retval None
 */
void IdleEnterTask(void *argument)
{ // 进入空闲状态
	uint8_t Idlestr = 0;
	uint8_t IdleBreakstr = 0;
	while (1)
	{
		// light get dark
		if (xQueueReceive(Idle_MessageQueue, &Idlestr, 1) == pdPASS)
		{						// 队列取值，无论取到什么（0或1），都会执行占空比设置	// 而且也没有代码让值变为1
			LCD_Set_Light(10); 	// 设置占空比为 5%
		}
		
		// 如果灯光变暗，并且按键或触摸屏幕打破了空闲状态，则恢复灯光。
		if (xQueueReceive(IdleBreak_MessageQueue, &IdleBreakstr, 1) == pdPASS)
		{										// 离开空闲状态时
			IdleTimerCount = 0;					// 屏幕唤醒总时长清零
			LCD_Set_Light(ui_LightSliderValue); // 将暗了的屏幕调为 用户设置的亮度值
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

/**
 * @brief  进入停机模式与恢复
 * @param  argument: Not used
 * @retval None
 */
void StopEnterTask(void *argument)
{
	uint8_t Stopstr;
	uint8_t HomeUpdataStr;
	uint8_t Wrist_Flag = 0;
	while (1)
	{
		if (xQueueReceive(Stop_MessageQueue, &Stopstr, 0) == pdPASS)
		{
		/*************************** 进入睡眠模式之前的操作 ***************************/
		sleep:					// goto 标号，c语言中的语法
			IdleTimerCount = 0; // 唤醒时间计数值清零

			// 传感器
			// 这里原作者没有对传感器进入睡眠模式的操作。

			// lcd
			LCD_RES_Clr();	   // 重置
			LCD_Close_Light(); // 关闭背灯
			// touch
			CST816_Sleep(); // 触摸模块睡眠

			/***********************************************************************************/

			/****************************** enter wakeup operations *****************************/

			esp_light_sleep_start(); // 进入STOP停机模式 ESP32进入停机不需要关闭调度器和 Systick

			// 运行到上面这一行STOP函数后，CPU就停止工作了，只有等待MCU被唤醒（如按下电源开关）才会执行下面的代码

			// here is the sleep period

			/***********************************************************************************/

			/****************************** quit wakeup operations *****************************/

			// 恢复运行模式

			/***********************************************************************************/

			/****************************** your wakeup operations *****************************/

			// MPU Check
			if (HWInterface.IMU.wrist_is_enabled)
			{
				uint8_t hor;
				hor = MPU_isHorizontal(); // 水平状态 ? 1 ： 0

				// 姿态过程变化判断
				if (hor && HWInterface.IMU.wrist_state == WRIST_DOWN)
				{											// 从手腕放下状态 -> 到水平 ： 说明手腕抬起，亮屏，正常运行
					HWInterface.IMU.wrist_state = WRIST_UP; // 更新姿态 为手腕抬起
					Wrist_Flag = 1;
					// resume, go on
				}
				else if (!hor && HWInterface.IMU.wrist_state == WRIST_UP)
				{											  // 从手腕处于水平 -> 到放下 ： 说明手腕放下，垂腕，熄屏，低功耗模式
					HWInterface.IMU.wrist_state = WRIST_DOWN; // 更新姿态 为垂腕放下
					IdleTimerCount = 0;
					goto sleep;
				}
			}

			// KEY1 ：交互键，返回上一页面（下降沿触发）；	KEY2 ：电源键（上升沿触发）
			// 按下KEY1 | 按下KEY2 | 充电 | 手腕抬起
			if (!KEY1 || KEY2 || HardInt_Charg_flag || Wrist_Flag)
			{
				Wrist_Flag = 0; // 清除标志位，以免一直亮屏运行
								// resume, go on
			}
			else
			{
				IdleTimerCount = 0; // 继续低功耗运行，睡眠
				goto sleep;
			}

			
			// lcd
			LCD_Init();				// 亮屏	
			LCD_Set_Light(ui_LightSliderValue);	
			// touch
			CST816_Wakeup();
			// check if is Charging
			if (ChargeCheck())
			{
				HardInt_Charg_flag = 1;	// 正充电呢
			}
			// send the Home Updata message
			xQueueSendToBack(HomeUpdata_MessageQueue, &HomeUpdataStr, 1);		// 发送主页面更新消息

			/**************************************************************************************/
		}
		osDelay(100);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

// 0.1秒调用一次
void IdleTimerCallback(void *argument)
{
	IdleTimerCount += 1; // 屏幕唤醒总时长
	// make sure the LightOffTime<TurnOffTime
	if (IdleTimerCount == (ui_LTimeValue * 10)) // ui_LTimeValue ： 常量时间
	{
		uint8_t Idlestr = 0; // 传入消息队列的 内容
		// send the Light off message
		// Idle_MessageQueue：传入哪个消息队列；Idlestr：传入内容；0：优先级；1：队列满，阻塞后发送消息等待时间（1个时钟周期）由于这个回调函数100ms就会回调一次，所以设置1Tick也没事
		xQueueSendToBack(Idle_MessageQueue, &Idlestr, 1); // 让屏幕变暗
	}
	if (IdleTimerCount == (ui_TTimeValue * 10))
	{
		uint8_t Stopstr = 1;
		IdleTimerCount = 0; // 计数值清零
		// send the Stop message
		xQueueSendToBack(Stop_MessageQueue, &Stopstr, 1); // 让屏幕变暗
	}
}
