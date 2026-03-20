#include <stdio.h>
#include "user_taskInit.h"

#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_flash.h"
#include "esp_log.h"


#define TAG     "MAIN"

TaskHandle_t defaultTaskHandle;

void app_main(void)
{

    // 初始化任务
    User_Tasks_Init();
    

    while (1)
    {
        // lv_task_handler();
    }
}