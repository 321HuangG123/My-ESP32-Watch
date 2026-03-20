#ifndef __USER_RUNMODETASKS_H__
#define __USER_RUNMODETASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "user_taskInit.h"
#include <stdint.h>

void IdleEnterTask(void *argument);
void StopEnterTask(void *argument);
void IdleTimerCallback(void *argument);

extern uint16_t IdleTimerCount;

#ifdef __cplusplus
}
#endif

#endif

