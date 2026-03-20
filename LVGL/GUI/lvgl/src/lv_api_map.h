/**
 * @file lv_api_map.h
 *
 */

#ifndef LV_API_MAP_H
#define LV_API_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl.h"

/*********************
 *      DEFINES
 *********************/

#define LV_NO_TASK_READY        LV_NO_TIMER_READY
#define LV_INDEV_STATE_REL      LV_INDEV_STATE_RELEASED
#define LV_INDEV_STATE_PR       LV_INDEV_STATE_PRESSED
#define LV_OBJ_FLAG_SNAPABLE    LV_OBJ_FLAG_SNAPPABLE   /*Fixed typo*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
// inline: 内联函数。编译器在编译时，会把函数体里的内容直接“复制”到调用它的地方
// LV_ATTRIBUTE_TIMER_HANDLER: 宏，它通常定义在 lv_conf.h 或底层头文件中。它的存在是为了处理不同编译器和平台的特殊需求。
static inline LV_ATTRIBUTE_TIMER_HANDLER uint32_t lv_task_handler(void)
{
    // 实现兼容，如果有人调用老版的 lv_task_handler，就直接去跑新版的 lv_timer_handler
    return lv_timer_handler();
}

/**********************
 *      MACROS
 **********************/


/**********************
 * INLINE FUNCTIONS
 **********************/

/**
 * Move the object to the foreground.
 * It will look like if it was created as the last child of its parent.
 * It also means it can cover any of the siblings.
 * @param obj       pointer to an object
 */
static inline void lv_obj_move_foreground(lv_obj_t * obj)
{
    lv_obj_t * parent = lv_obj_get_parent(obj);
    lv_obj_move_to_index(obj, lv_obj_get_child_cnt(parent) - 1);
}

/**
 * Move the object to the background.
 * It will look like if it was created as the first child of its parent.
 * It also means any of the siblings can cover the object.
 * @param obj       pointer to an object
 */
static inline void lv_obj_move_background(lv_obj_t * obj)
{
    lv_obj_move_to_index(obj, 0);
}



/**********************
 * DEPRECATED FUNCTIONS
 **********************/

static inline uint32_t lv_obj_get_child_id(const struct _lv_obj_t * obj)
{
    LV_LOG_WARN("lv_obj_get_child_id(obj) is deprecated, please use lv_obj_get_index(obj).");
    return lv_obj_get_index(obj);
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_API_MAP_H*/
