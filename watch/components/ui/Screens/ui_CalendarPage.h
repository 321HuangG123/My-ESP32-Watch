#ifndef _UI_CALENDARPAGE_H
#define _UI_CALENDARPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/src/core/lv_obj.h"
#include "PageManager.h"

extern lv_obj_t * ui_CalendarPage;
extern lv_obj_t * ui_CalendarPageCalendar;

extern Page_t Page_Calender;

void ui_CalendarPage_screen_init(void);
void ui_CalendarPage_screen_deinit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
