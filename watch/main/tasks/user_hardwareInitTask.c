#include "user_hardwareInitTask.h"
#include "pwm.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_power.h"
#include "hwDataAccess.h"
#include "ui_DateTimeSetPage.h"
#include "DataSave.h"

#include "lcd.h"
#include "lcd_init.h"
#include "CST816.h"


/**
 * @brief  硬件初始化
 * @param  argument: Not used
 * @retval None
 */
void HardwareInitTask(void *argument)
{
  while (1)
  {
    /*
      STM32源代码里需要这样挂起调度器，但是在这里会报错。 
      vTaskSuspendAll() 会挂起调度器。在调度器挂起期间，所有的任务切换都停止了，这意味着“时间（Tick）”也停止了。

      FreeRTOS 规定：当调度器被挂起时，你绝对不能调用任何会导致阻塞（带有 xTicksToWait > 0）的函数。
      因为调度器停了，没人能把你唤醒。

      ESP-IDF 的 ledc_timer_config 驱动内部为了保证多线程安全，会使用互斥锁（Mutex/Semaphore）。
      锁的操作包含阻塞逻辑，从而触发了系统的安全检查。
    */
    // vTaskSuspendAll();   // 暂停调度器，让当前任务能够连续执行一段代码而不被任务切换打断
    
    // 开启PWM方波 --> 控制背光亮度
    // 1. 配置 LEDC 定时器
    ledc_timer_config_t timer_conf = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz         = 1000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    // 2. 配置 LEDC 通道（绑定 GPIO11）
    ledc_channel_config_t channel_conf = {
        .gpio_num   = GPIO_NUM_11,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 8191,  // 100% 占空比 (8191 / 8191 = 1)
        .hpoint     = 0,
    };
    init_pwm_on_gpiox(timer_conf, channel_conf);


    // 初始化ADC
    // 这里用 ADC1 的通道0(GPIO 1)来进行初始化
    Power_ADC_Init(BAT_ADC_CHAN);


    // 电源模块初始化
    HWInterface.Power.Init();


    // 按键初始化
    Key_Port_Init();            


    /*****************
     *    传感器部分
     *****************/ 
    // 温湿度传感器初始化
    uint8_t num = 3;
    while (num && HWInterface.DHT11.ConnectionError) // ConnectionError初始化是 1
    { // 通过重复尝试，达到初始化的目的。因为有时候硬件没有准备好可能会初始化失败
      num--;
      HWInterface.DHT11.ConnectionError = HWInterface.DHT11.Init(); // 初始化I2C以及往AHT21模块写入数据启动模块
    }
    
    // 电子指南针模块初始化
    num = 3;
    while (num && HWInterface.Ecompass.ConnectionError)
    {
      num--;
      HWInterface.Ecompass.ConnectionError = HWInterface.Ecompass.Init();
    }
    if (!HWInterface.Ecompass.ConnectionError)
      HWInterface.Ecompass.Sleep();       // 进入睡眠模式，省电
    
    // 气压计初始化
    num = 3;
    while (num && HWInterface.Barometer.ConnectionError)
    { 
      num--;
      HWInterface.Barometer.ConnectionError = HWInterface.Barometer.Init();
    }

    // MPU6050初始化
    num = 3;
    while (num && HWInterface.IMU.ConnectionError)
    { 
      num--;
      HWInterface.IMU.ConnectionError = HWInterface.IMU.Init();
    } // 其他模块有进入睡眠模式，但可能考虑到手表在低功耗模式下也得计算步数，作者就没有让该模块睡眠

    // 心率模块初始化
    num = 3;
    while (num && HWInterface.HR_meter.ConnectionError)
    { 
      num--;
      HWInterface.HR_meter.ConnectionError = HWInterface.HR_meter.Init();
    }
    if (!HWInterface.HR_meter.ConnectionError)
      HWInterface.HR_meter.Sleep();   // 心率模块进入睡眠模式 

    // EEPROM
    EEPROM_Init();
    if (!EEPROM_Check())  // 0 : 1 ? ok : err
    { // 如果正常，则获取断电记忆与数据恢复，从E2PROM中获取保存过的数据
      uint8_t recbuf[3];
      SettingGet(recbuf, 0x10, 2);
      if ((recbuf[0] != 0 && recbuf[0] != 1) || (recbuf[1] != 0 && recbuf[1] != 1))
      {
        HWInterface.IMU.wrist_is_enabled = 0;
        ui_APPSy_EN = 0;
      }
      else
      {
        HWInterface.IMU.wrist_is_enabled = recbuf[0];
        ui_APPSy_EN = recbuf[1];
      }

      time_t now;
      struct tm timeinfo;
      time(&now);                // 获取当前时间戳
      localtime_r(&now, &timeinfo); // 将时间戳转换为本地日期结构

      uint8_t current_day = (uint8_t)timeinfo.tm_mday;

      SettingGet(recbuf, 0x20, 3);
      if (recbuf[0] == current_day)
      {
        uint16_t steps = 0;
        steps = recbuf[1] & 0x00ff;
        steps = steps << 8 | recbuf[2];
        if (!HWInterface.IMU.ConnectionError)
          dmp_set_pedometer_step_count((unsigned long)steps);
      }
    }
    
    // touch
    CST816_GPIO_Init();
    CST816_RESET();

    // lcd
    LCD_Init();
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
    vTaskDelay(pdMS_TO_TICKS(10));
    LCD_Set_Light(50);
    LCD_ShowString(72, LCD_H / 2, (uint8_t *)"Welcome!", WHITE, BLACK, 24, 0); // 12*6,16*8,24*12,32*16
    uint8_t lcd_buf_str[17];
    LCD_ShowString(34, LCD_H / 2 + 48, (uint8_t *)lcd_buf_str, WHITE, BLACK, 24, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    LCD_Fill(0, LCD_H / 2 - 24, LCD_W, LCD_H / 2 + 49, BLACK);

    // ui
    // LVGL init
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    ui_init();

    // xTaskResumeAll();  // 恢复调度器
    vTaskDelete(NULL); // 杀死任务
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
