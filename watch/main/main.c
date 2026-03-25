#include <stdio.h>
#include "user_taskInit.h"

#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_flash.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#define TAG     "MAIN"

TaskHandle_t defaultTaskHandle;

void app_main(void)
{

    esp_task_wdt_config_t config = {
        .timeout_ms = 5000,
        .idle_core_mask = (1 << 0), // 同时监控 CPU 0 的空闲任务
        .trigger_panic = true,
    };
    esp_task_wdt_init(&config);

    // 初始化任务
    User_Tasks_Init();
    

    while (1)
    {
        // lv_task_handler();
    }
}