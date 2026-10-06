#pragma once

#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/* vTaskDelay is declared in FreeRTOS.h for this fake.
 * The sim does not run extra tasks. Silicon does. */

static inline BaseType_t xTaskCreate(TaskFunction_t task, const char *name,
                                     const uint32_t stack, void *arg,
                                     UBaseType_t priority, TaskHandle_t *out)
{
    (void)task;
    (void)name;
    (void)stack;
    (void)arg;
    (void)priority;
    (void)out;
    return pdPASS;
}

#ifdef __cplusplus
}
#endif
