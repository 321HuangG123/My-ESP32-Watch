#ifndef __USER_TASKINIT_H__
#define __USER_TASKINIT_H__

// 初始化任务
void User_Tasks_Init(void);

// lvgl处理任务
void LvHandlerTask(void *argument);

// 喂狗任务
void WDOGFeedTask(void *argument);

extern QueueHandle_t Key_MessageQueue;
extern QueueHandle_t Idle_MessageQueue;
extern QueueHandle_t Stop_MessageQueue;
extern QueueHandle_t IdleBreak_MessageQueue;
extern QueueHandle_t HomeUpdata_MessageQueue;
extern QueueHandle_t DataSave_MessageQueue;

#endif
