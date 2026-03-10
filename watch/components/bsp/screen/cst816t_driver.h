#ifndef _CST816T_DRIVER_H_
#define _CST816T_DRIVER_H_
#include "driver/gpio.h"
#include "esp_err.h"

#define GestureID 0x01     // 手势寄存器
#define FingerNum 0x02     // 手指数量
#define XposH 0x03         // x高四位
#define XposL 0x04         // x低八位
#define YposH 0x05         // y高四位
#define YposL 0x06         // y低八位
#define ChipID 0xA7        // 芯片型号
#define MotionMask 0xEC    // 触发动作
#define AutoSleepTime 0xF9 // 自动休眠
#define IrqCrl 0xFA        // 中断控制
#define AutoReset 0xFB     // 无手势休眠
#define LongPressTime 0xFC // 长按休眠
#define DisAutoSleep 0xFE  // 使能低功耗模式

//CST816T 触摸IC驱动

typedef struct 
{
    gpio_num_t  scl;     //SCL管脚
    gpio_num_t  sda;     //SDA管脚
    uint32_t    fre;       //I2C速率
    uint16_t    x_limit;    //X方向触摸边界
    uint16_t    y_limit;    //y方向触摸边界
}cst816t_cfg_t;


/** CST816T初始化
 * @param cfg 配置
 * @return err
*/
esp_err_t   cst816t_init(cst816t_cfg_t* cfg);

/** 读取坐标值
 * @param  x x坐标
 * @param  y y坐标
 * @param state 松手状态 0,松手 1按下
 * @return 无
*/
void cst816t_read(int16_t *x,int16_t *y,int *state);

#endif
