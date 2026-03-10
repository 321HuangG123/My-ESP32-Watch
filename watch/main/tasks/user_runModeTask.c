#include "user_runModeTask.h"
// #include "user_tasksInit.h"
#include "stdio.h"

#include "esp_log.h"


#define	TAG "USER_RUNMODETASK"		

// 0.1秒调用一次
void IdleTimerCallback(void *argument)
{
	// IdleTimerCount += 1; // 屏幕唤醒总时长
	// // make sure the LightOffTime<TurnOffTime
	// if (IdleTimerCount == (ui_LTimeValue * 10)) // ui_LTimeValue ： 常量时间
	// {
	// 	uint8_t Idlestr = 0; // 传入消息队列的 内容
	// 	// send the Light off message
	// 	// Idle_MessageQueue：传入哪个消息队列；Idlestr：传入内容；0：优先级；1：队列满，阻塞后发送消息等待时间（1个时钟周期）由于这个回调函数100ms就会回调一次，所以设置1Tick也没事
	// 	osMessageQueuePut(Idle_MessageQueue, &Idlestr, 0, 1); // 让屏幕变暗
	// }
	// if (IdleTimerCount == (ui_TTimeValue * 10))
	// {
	// 	uint8_t Stopstr = 1;
	// 	IdleTimerCount = 0; // 计数值清零
	// 	// send the Stop message
	// 	osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1); // 无操作，关屏
	// }

	printf("======IdleTimerCallback run======\r\n");
}