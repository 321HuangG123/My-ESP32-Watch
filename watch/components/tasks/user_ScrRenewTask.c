/* Private includes -----------------------------------------------------------*/
// includes
#include "user_taskInit.h"
#include "user_ScrRenewTask.h"
#include "lvgl.h"
#include "Screens/ui_HomePage.h"
#include "Screens/ui_MenuPage.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lsm303/lsm303.h"
#include "spl06_001/spl06_001.h"

#include "esp_task_wdt.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern QueueHandle_t Key_MessageQueue;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  当按键按下时，屏幕刷新
 * @param  argument: Not used
 * @retval None
 */
void ScrRenewTask(void *argument)
{	
	esp_task_wdt_add(NULL);         // 监控当前任务
	uint8_t keystr = 0;
	while (1)
	{		
		if (xQueueReceive(Key_MessageQueue, &keystr, 0) == pdPASS)	// 从按键消息队列中获取消息，非阻塞式
		{	// 如果有按键按下
			// key1 pressed
			if (keystr == 1)
			{
				Page_Back();		// 页面返回
				if (Page_Get_NowPage()->page_obj == &ui_MenuPage)	// 页面返回后，当前的页面是主页从左往右划的菜单页面
				{
					// 下面的操作都是为了低功耗
					// EM7028_hrs_DisEnable();		// 心率血氧模块睡眠			
					LSM303DLH_Sleep();			// 电子罗盘（指南针）
					SPL_Sleep();				// 气压
				}
			}

			// key2 pressed
			else if (keystr == 2)
			{
				Page_Back_Bottom();				// 栈出到最底部（homePage）为止

				// 低功耗
				// EM7028_hrs_DisEnable();		
				LSM303DLH_Sleep();
				SPL_Sleep();					
			}
		}
		esp_task_wdt_reset();
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
