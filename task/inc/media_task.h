#ifndef MEDIA_TASK_H
#define MEDIA_TASK_H
#include "gd32f4xx.h"
#include "app_su03t.h"

void MediaTask(void *argument);

void Su03tTask(void *argument);

void app_su03t_proc(Su03tData *data);
void app_su03tControl(uint8_t mode, uint8_t adjust);

#endif /* MEDIA_TASK_H */
