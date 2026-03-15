#ifndef _UI_ABOUTPAGE_H
#define _UI_ABOUTPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/src/core/lv_obj.h"
#include "PageManager.h"

extern lv_obj_t * ui_AboutPage;

extern Page_t Page_About;

void ui_AboutPage_screen_init(void);
void ui_AboutPage_screen_deinit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
