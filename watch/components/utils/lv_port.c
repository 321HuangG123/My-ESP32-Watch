#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "esp_log.h"
#include "screen/st7789_driver.h"
#include "screen/cst816t_driver.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// 定义屏幕的�?�度和高�?
#define LCD_WIDTH   240
#define LCD_HEIGHT  284

#define TAG "lv_port"

static lv_disp_drv_t disp_drv;

void disp_flush(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    st7789_flush(area->x1, area->x2 + 1, area->y1, area->y2 + 20 + 1, color_p);
}

// 1. 初�?�化和注册LVGL显示驱动
void lv_disp_init(void)
{
    // 定义显示缓存 lvgl库�?�算出�?�显示的内�?�画�?，就存放在这�?缓存�?
    static lv_disp_draw_buf_t disp_buf;
    const size_t disp_buf_size = LCD_WIDTH * (LCD_HEIGHT/7);

    // MALLOC_CAP_INTERNAL  :   从内部RAM�?申�?�内�?
    // MALLOC_CAP_DMA       :   �?用于DMA传输 
    lv_color_t * disp1 = heap_caps_malloc(disp_buf_size * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    lv_color_t * disp2 = heap_caps_malloc(disp_buf_size * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!disp1 || !disp2)
    {
        ESP_LOGE(TAG, "disp buff malloc fail!");
        return;
    }
    // 显示缓存
    lv_disp_draw_buf_init(&disp_buf, disp1, disp2, disp_buf_size);

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = disp_flush;
    lv_disp_drv_register(&disp_drv);
}

// IRAM_ATTR : 由于该函数�?�繁调用，因此将它放在内存里，运行速度更快
void IRAM_ATTR indev_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    int16_t x, y;
    int state;
    cst816t_read(&x, &y, &state);
    data->point.x = x; 
    data->point.y = y; 
    data->state = state;
}

// 2. 初�?�化和注册LVGL触摸驱动
void lv_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read;
    lv_indev_drv_register(&indev_drv);
}

// st7789 回调函数
void lv_flush_done_cb(void *param)
{
    lv_disp_flush_ready(&disp_drv);
}

// 3. 初�?�化ST7789�?件接�?
void st7789_hw_init(void)
{   
    // SPI 引脚初�?�化
    st7789_cfg_t st7789_config = {
        .cs         = GPIO_NUM_10,
        .clk        = GPIO_NUM_6,
        .mosi       = GPIO_NUM_7,
        .bl         = GPIO_NUM_11,    
        .dc         = GPIO_NUM_17,
        .rst        = GPIO_NUM_3,
        .spi_fre    = 40*1000*1000,
        .height     = LCD_HEIGHT,
        .width      = LCD_WIDTH,
        .spin       = 0,
        .done_cb    = lv_flush_done_cb,
        .cb_param   = &disp_drv,
    };

    st7789_driver_hw_init(&st7789_config);
}

// 4. 初�?�化CST816T�?件接�?
void cst816t_hw_init(void)
{
    // I2C 引脚初�?�化
    cst816t_cfg_t cst816t_config = {
        .scl     = GPIO_NUM_5,
        .sda     = GPIO_NUM_4,
        .fre     = 300*1000,
        .x_limit = LCD_WIDTH,
        .y_limit = LCD_HEIGHT,
    };
    cst816t_init(&cst816t_config);
}

// 初�?�化lvgl定时�? �? 回调函数
void lv_timer_cb(void* arg)
{
    uint32_t tick_interval = *((uint32_t *)arg);
    lv_tick_inc(tick_interval);
}

// 5. 提供一�?定时器给LVGL使用
void lv_tick_init(void)
{
    // 定时器间隔时�?
    static uint32_t tick_interval = 5;
    const esp_timer_create_args_t arg = {
        .arg                    = &tick_interval,
        .callback               = lv_timer_cb,
        .name                   = "",
        .dispatch_method        = ESP_TIMER_TASK,      // 定时器回调函数在�?里�?�理：任�? / �?�?
        .skip_unhandled_events  = true,                // 有�?�个任务阻�?�了定时器回调函数的调用，当执�?�时，会跳过前面已经超时的了回调函数 / 反之则会执�?�之前超时的回调函数
    };

    esp_timer_handle_t timer_handle;
    esp_timer_create(&arg, &timer_handle);
    esp_timer_start_periodic(timer_handle, tick_interval * 1000);
}

void lv_port_init(void)
{
    // lvgl库自己的初�?�化
    lv_init();
    // ST7789�?件初始化
    st7789_hw_init();
    // CST816T�?件初始化
    cst816t_hw_init();
    // LVGL 显示初�?�化
    lv_disp_init();
    // LVGL 触摸初�?�化
    lv_indev_init();
    // LVGL 定时器初始化
    lv_tick_init();
}
