/* Private includes -----------------------------------------------------------*/
//includes
#include "user_taskInit.h"
#include "Screens/ui_HomePage.h"
#include "key/key.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  按键检测任务(中断只负责唤醒，任务才负责按键扫描)
  * @param  argument: Not used
  * @retval None
  */
void KeyTask(void *argument)
{
	uint8_t keystr=0;
	uint8_t Stopstr=0;
	uint8_t IdleBreakstr=0;
	while(1)
	{
		switch(KeyScan(0))
		{
			case 1:		// KEY1：返回上一页面
				keystr = 1;
				xQueueSendToBack(Key_MessageQueue, &keystr, 1);	// 告知是key1还是key2触发
				xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 1);	// 无论哪个键按下都会接触空闲状态
				break;

			case 2:		// KEY2：电源
				if(Page_Get_NowPage()->page_obj == &ui_HomePage)	// 当前页面处于主页
				{	
					xQueueSendToBack(Stop_MessageQueue, &Stopstr, 1);		// 进入停机状态
				}
				else
				{
					keystr = 2;
					xQueueSendToBack(Key_MessageQueue, &keystr, 1);	 // 告知是key1还是key2触发
					xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 1);	// 无论哪个键按下都会接触空闲状态
				}
				break;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
