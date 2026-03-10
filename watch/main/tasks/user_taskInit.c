#include "user_taskInit.h"
#include "user_runModeTask.h"
#include "user_hardwareInitTask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#include "esp_log.h"

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
    // HardwareInitTaskHandle = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes);

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
    res = xTaskCreatePinnedToCore(HardwareInitTask, "HardwareInitTask", 4096 * 3, NULL, 17, &HardwareInitTaskHandle, tskNO_AFFINITY);
    if (res != pdPASS)
    {
        ESP_LOGI(TAG, "HardwareInitTask Creation Failed.........");
    }
    
    // LvHandlerTaskHandle = osThreadNew(LvHandlerTask, NULL, &LvHandlerTask_attributes);
    // WDOGFeedTaskHandle = osThreadNew(WDOGFeedTask, NULL, &WDOGFeedTask_attributes);
    // IdleEnterTaskHandle = osThreadNew(IdleEnterTask, NULL, &IdleEnterTask_attributes);
    // StopEnterTaskHandle = osThreadNew(StopEnterTask, NULL, &StopEnterTask_attributes);
    // KeyTaskHandle = osThreadNew(KeyTask, NULL, &KeyTask_attributes);
    // ScrRenewTaskHandle = osThreadNew(ScrRenewTask, NULL, &ScrRenewTask_attributes);
    // SensorDataTaskHandle = osThreadNew(SensorDataUpdateTask, NULL, &SensorDataTask_attributes);
    // HRDataTaskHandle = osThreadNew(HRDataUpdateTask, NULL, &HRDataTask_attributes);
    // ChargPageEnterTaskHandle = osThreadNew(ChargPageEnterTask, NULL, &ChargPageEnterTask_attributes);
    // MessageSendTaskHandle = osThreadNew(MessageSendTask, NULL, &MessageSendTask_attributes);
    // MPUCheckTaskHandle = osThreadNew(MPUCheckTask, NULL, &MPUCheckTask_attributes);
    // DataSaveTaskHandle = osThreadNew(DataSaveTask, NULL, &DataSaveTask_attributes);



    // LvHandlerTaskHandle;
    // WDOGFeedTaskHandle;
    // IdleEnterTaskHandle;
    // StopEnterTaskHandle;
    // KeyTaskHandle;
    // ScrRenewTaskHandle;
    // SensorDataTaskHandle;
    // HRDataTaskHandle;
    // ChargPageEnterTaskHandle;
    // MessageSendTaskHandle;
    // MPUCheckTaskHandle;
    // DataSaveTaskHandle;
    

    
    // uint8_t HomeUpdataStr;
    // osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);   // 发送主页面更新消息
}