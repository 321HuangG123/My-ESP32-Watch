/* Private includes -----------------------------------------------------------*/
// includes
#include "user_taskInit.h"
#include "user_ScrRenewTask.h"
#include "main.h"
#include "lvgl.h"
#include "ui_HomePage.h"
#include "ui_MenuPage.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern osMessageQueueId_t Key_MessageQueue;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  当按键按下时，屏幕刷新
 * @param  argument: Not used
 * @retval None
 */
void ScrRenewTask(void *argument)
{
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
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
