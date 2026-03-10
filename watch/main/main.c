#include <stdio.h>
#include "screen/st7789_driver.h"
#include "utils/lv_port.h"
#include "user_taskInit.h"

#include "lvgl.h"
#include "lv_demos.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_flash.h"
#include "esp_log.h"


#define TAG     "MAIN"

TaskHandle_t defaultTaskHandle;

void app_main(void)
{
    // // lvgl初始化
    // lv_port_init();

    // // 开启背光
    // st7789_lcd_backlight(1);
    
    // ESP-IDF会默认开启Watchdog，得在后续的任务中定期喂狗，否则会被系统重启

    // 初始化任务
    User_Tasks_Init();
    

    while (1)
    {
        lv_task_handler();
    }
}