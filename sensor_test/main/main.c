#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "max30102_read.h"
#include "max30102.h"
#include "esp_task_wdt.h"

#define TAG "MAIN"

// 心率血氧模块所需的变量
int32_t hrAvg;                          // 心率
int32_t spo2Avg;                        // 血氧浓度



// 主函数
void app_main(void)
{
    
    Init_MAX30102();  // 初始化MAX30102心率血氧传感器

    vTaskDelay(pdMS_TO_TICKS(1000));

    uint8_t part_id = 0;
    // 调用驱动中的读取 ID 函数，内部读取寄存器 REG_PART_ID (0xFF)
    if (maxim_max30102_read_id(&part_id)) {
        if (part_id == 0x15) {
            ESP_LOGI(TAG, ">> [SUCCESS] I2C Communication OK! Part ID: 0x%02X", part_id);
        } else {
            ESP_LOGW(TAG, ">> [WARNING] I2C Connected, but ID Mismatch! Expected 0x11, got 0x%02X", part_id);
        }
    } else {
        ESP_LOGE(TAG, ">> [ERROR] I2C Communication Failed!");
        
        // 如果 ID 都读不到，直接进入死循环，防止跑后续错误的逻辑
        while(1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 登记看门狗
    esp_task_wdt_add(NULL);

    while(1)
    {
        // 读取心率和血氧数据并显示
        ReadHeartRateSpO2();  //读取心率血氧

        ESP_LOGI(TAG, "Heart Rate: %d bpm, SpO2: %d%%", hrAvg, spo2Avg);

        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(100));  // 延时0.1秒
    }
}
