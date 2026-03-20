/* Private includes -----------------------------------------------------------*/
// includes
#include "user_taskInit.h"
#include "user_ScrRenewTask.h"
#include "user_runModeTask.h"
#include "Screens/ui_HomePage.h"
#include "Screens/ui_ChargPage.h"
#include "hwDataAccess.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

// 充电的中断标志位
uint8_t HardInt_Charg_flag = 0;

// ChargeCheck() 在源代码中是通过GPIO口检测电平判断的，这里我们模拟就好了
bool ChargeCheck(void)
{
	// 模拟充电检测逻辑
	return false; // 默认不充电
}

/**
 * @brief  charg page enter task
 * @param  argument: Not used
 * @retval None
 */
void ChargPageEnterTask(void *argument)
{
	while (1)
	{
		// 硬件中断发生
		if (HardInt_Charg_flag) // 当正在充电时标志位置1
		{
			IdleTimerCount = 0;
			HardInt_Charg_flag = 0; // 清除标志位
			if ((ChargeCheck()) && (Page_Get_NowPage()->page_obj != &ui_ChargPage))	// 正在充电且当前页面不是充电页面
			{
				Page_Load(&Page_Charg);	// 加载充电页面
			}
			else if ((!ChargeCheck()) && (Page_Get_NowPage()->page_obj == &ui_ChargPage))	// 拔掉充电器且位于充电页面
			{
				Page_Back();	// 页面返回
			}
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
