#pragma once

#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/* vTaskDelay is declared in FreeRTOS.h for this fake.
 * Each task is a host thread. Priority and stack size are ignored. */
BaseType_t xTaskCreate(TaskFunction_t task, const char *name,
                       const uint32_t stack, void *arg,
                       UBaseType_t priority, TaskHandle_t *out);

#ifdef __cplusplus
}
#endif
