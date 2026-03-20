#include "user_taskInit.h"
#include "user_runModeTask.h"
#include "user_hardwareInitTask.h"
#include "user_KeyTask.h"
#include "user_ScrRenewTask.h"
#include "user_SensUpdateTask.h"
#include "user_ChargCheckTask.h"
#include "user_MessageSendTask.h"
#include "user_DataSaveTask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "lvgl/src/core/lv_disp.h"
#include "esp_task_wdt.h"


#define TAG "USER_TASKINIT"

TimerHandle_t IdleTimerHandle;

QueueHandle_t Key_MessageQueue;
QueueHandle_t Idle_MessageQueue;
QueueHandle_t Stop_MessageQueue;
QueueHandle_t IdleBreak_MessageQueue;
QueueHandle_t HomeUpdata_MessageQueue;
QueueHandle_t DataSave_MessageQueue;

TaskHandle_t HardwareInitTaskHandle;
TaskHandle_t LvHandlerTaskHandle;
TaskHandle_t WDOGFeedTaskHandle;
TaskHandle_t IdleEnterTaskHandle;
TaskHandle_t StopEnterTaskHandle;
TaskHandle_t KeyTaskHandle;
TaskHandle_t ScrRenewTaskHandle;
TaskHandle_t SensorDataTaskHandle;
TaskHandle_t HRDataTaskHandle;
TaskHandle_t ChargPageEnterTaskHandle;
TaskHandle_t MessageSendTaskHandle;
TaskHandle_t MPUCheckTaskHandle;
TaskHandle_t DataSaveTaskHandle;

// 初始化任务
void User_Tasks_Init(void)
{
    // 创建一个周期性的Timer，每100ms调用一次 IdleTimerCallback 回调函数
    // 定时器的空间很小，不要在回调中随便写ESP_LOGI等这种复杂的操作，会容易溢出
    IdleTimerHandle = xTimerCreate("IdleTimer", pdMS_TO_TICKS(100), pdTRUE, NULL, (TimerCallbackFunction_t)IdleTimerCallback);
    xTimerStart(IdleTimerHandle, 100);

    // 消息队列创建。队列长度为1，消息大小为1Byte即uint8_t
    Key_MessageQueue = xQueueCreate(1, 1);              // 按键按下分别执行什么事件
    Idle_MessageQueue = xQueueCreate(1, 1);             // 空闲状态检测
    Stop_MessageQueue = xQueueCreate(1, 1);             // 进入停机模式 省电
    IdleBreak_MessageQueue = xQueueCreate(1, 1);        // 解除空闲状态模式，有用户操作
    HomeUpdata_MessageQueue = xQueueCreate(1, 1);       // 主页数据更新
    DataSave_MessageQueue = xQueueCreate(2, 1);         // 保存 或 更新数据

    // 创建手表所需要的各个任务
    /*
        #define configMAX_PRIORITIES  ( 25 )
        osPriorityIdle	        1	        0	        空闲任务级别
        osPriorityLow	        8	        1 - 2	    调试日志打印、低速监控
        osPriorityBelowNormal	16	        3 - 4	    后台数据记录
        osPriorityNormal	    24	        5 - 9	    普通应用逻辑、GUI 任务
        osPriorityAboveNormal	32	        10 - 14	    协议栈处理、关键逻辑
        osPriorityHigh	        40	        15 - 19	    传感器采样、快速响应任务
        osPriorityRealtime	    48	        20 - 24	    非常紧急的硬实时任务

        Core 0 (PRO CPU)	负责运行 Wi-Fi 协议栈、蓝牙协议栈、系统定时器、中断处理。	 保持系统稳定，处理网络通讯。
        Core 1 (APP CPU)	相对比较空闲，通常用来跑用户的业务代码。	                运行 LVGL UI 刷新、复杂的业务逻辑运算。
    */
    // tskNO_AFFINITY: FreeRTOS 调度器会根据负载自动决定任务跑在哪个核上
    // 不要用error check宏包裹这个函数，会报错，因为这个函数返回值不是esp_err_t类型
    // ESP_ERROR_CHECK(xTaskCreatePinnedToCore(HardwareInitTask, "HardwareInitTask", 4096 * 3, NULL, 17, &HardwareInitTaskHandle, tskNO_AFFINITY));   
    BaseType_t res;
    // 创建硬件初始化任务，优先级设置为17，栈大小设置为4096*3字节
    res = xTaskCreatePinnedToCore(HardwareInitTask, "HardwareInitTask", 128 * 10, NULL, 17, &HardwareInitTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "HardwareInitTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(LvHandlerTask, "LvHandlerTask", 128 * 24, NULL, 2, &LvHandlerTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "LvHandlerTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(WDOGFeedTask, "WDOGFeedTask", 128 * 1, NULL, 16, &WDOGFeedTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "WDOGFeedTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(IdleEnterTask, "IdleEnterTask", 128 * 1, NULL, 15, &IdleEnterTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "IdleEnterTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(StopEnterTask, "StopEnterTask", 128 * 16, NULL, 16, &StopEnterTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "StopEnterTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(KeyTask, "KeyTask", 128 * 1, NULL, 5, &KeyTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "KeyTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(ScrRenewTask, "ScrRenewTask", 128 * 10, NULL, 2, &ScrRenewTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "ScrRenewTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(SensorDataUpdateTask, "SensorDataUpdateTask", 128 * 5, NULL, 2, &SensorDataTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "SensorDataUpdateTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(HRDataUpdateTask, "HRDataUpdateTask", 128 * 5, NULL, 16, &HRDataTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "HRDataUpdateTask Creation Failed.........");
    }
    
    res = xTaskCreatePinnedToCore(ChargPageEnterTask, "ChargPageEnterTask", 128 * 10, NULL, 16, &ChargPageEnterTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "ChargPageEnterTask Creation Failed.........");
    }
    
    // res = xTaskCreatePinnedToCore(MessageSendTask, "MessageSendTask", 128 * 5, NULL, 2, &MessageSendTaskHandle, tskNO_AFFINITY);
    // if (res != pdPASS)
    // {
    //     ESP_LOGI(TAG, "MessageSendTask Creation Failed.........");
    // }

    res = xTaskCreatePinnedToCore(MPUCheckTask, "MPUCheckTask", 128 * 3, NULL, 2, &MPUCheckTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "MPUCheckTask Creation Failed.........");
    }

    res = xTaskCreatePinnedToCore(DataSaveTask, "DataSaveTask", 128 * 5, NULL, 2, &DataSaveTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "DataSaveTask Creation Failed.........");
    }

    uint8_t HomeUpdataStr = 1;
    xQueueSendToBack(HomeUpdata_MessageQueue, &HomeUpdataStr, 1);   // 发送主页面更新消息
}

/**
 * @brief  LVGL Handler task, to run the lvgl
 * @param  argument: Not used
 * @retval None
 */
void LvHandlerTask(void *argument)
{ //
  uint8_t IdleBreakstr = 0;
  while (1)
  { // lv_disp_get_inactive_time(NULL) 获取自上次用户操作以来，已经过去了多少毫秒
    if (lv_disp_get_inactive_time(NULL) < 1000)         // 1秒内有用户操作，发送解除空闲状态消息，参数值为0
    {
      // Idle time break, set to 0
      xQueueSendToBack(IdleBreak_MessageQueue, &IdleBreakstr, 0); 
    }
    // 老版的叫 task_handler，新版的叫timer_handler，整个项目就只有这里调用了timer_handler
    // 是lvgl的“心脏”，lvgl系统必须定时执行这个函数，才不会变成UI页面无法正常交互的“砖”
    lv_task_handler();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


/**
 * @brief  Watch Dog Feed task
 * @param  argument: Not used
 * @retval None
 */
void WDOGFeedTask(void *argument)
{
  // 初始化Task Watchdog
  esp_task_wdt_init(5);     // 5秒超时
  esp_task_wdt_add(NULL);         // 监控当前任务
  while (1)
  {
    esp_task_wdt_reset();       // 喂狗
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
