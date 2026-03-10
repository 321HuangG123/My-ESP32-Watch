#include "key.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void IRAM_ATTR Key_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    // 这里可以根据不同的按键引脚号执行不同的操作
    if (gpio_num == KEY1_PIN) {
        // 处理 KEY1 按下事件
    } else if (gpio_num == KEY2_PIN) {
        // 处理 KEY2 按下事件
    }
}

void Key_Port_Init(void)
{
    gpio_config_t key1_gpio_t = {
        .intr_type = GPIO_INTR_NEGEDGE,          // 下降沿中断
        .mode = GPIO_MODE_INPUT,                 // 输入模式
        .pin_bit_mask = (1ull << KEY1_PIN), // 配置 KEY1 和 KEY2 引脚)
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&key1_gpio_t);

	gpio_config_t key2_gpio_t = {
        .intr_type = GPIO_INTR_POSEDGE,          // 上升沿中断
        .mode = GPIO_MODE_INPUT,                 // 输入模式
        .pin_bit_mask = (1ull << KEY2_PIN), // 配置 KEY1 和 KEY2 引脚)
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&key2_gpio_t);

    // 安装GPIO中断服务
    // ESP_INTR_FLAG_LEVEL1 代表优先级1（较低），ESP_INTR_FLAG_IRAM 代表在关闭 Flash 缓存时也能运行
    gpio_install_isr_service(ESP_INTR_FLAG_NMI | ESP_INTR_FLAG_IRAM);

    // 注册回调函数
    gpio_isr_handler_add(KEY1_PIN, Key_isr_handler, (void *)KEY1_PIN);
    gpio_isr_handler_add(KEY2_PIN, Key_isr_handler, (void *)KEY2_PIN);

}

uint8_t KeyScan(uint8_t mode)
{
	static uint8_t key_up = 1;
	static uint8_t key_down = 0;
	uint8_t keyvalue = 0;

	if(mode)
	{
		key_up = 1;
		key_down = 0;
	}

	if( key_up && ((!KEY1) || KEY2))
	{
		vTaskDelay(pdMS_TO_TICKS(5));//ensure the key
		if(!KEY1)
			key_down = 1;
		if(KEY2)
			key_down = 2;
		if(key_down) 
			key_up = 0;
	}

	if ( key_down && (KEY1 && (!KEY2)) )
	{
		vTaskDelay(pdMS_TO_TICKS(5));//ensure the key
		if(KEY1 && (!KEY2)) 
		{
			key_up = 1;
			keyvalue = key_down;
			key_down = 0;
		}
	}

	return keyvalue;
}
