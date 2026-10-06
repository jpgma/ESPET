#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t TickType_t;
typedef void *TaskHandle_t;
typedef int BaseType_t;
typedef unsigned UBaseType_t;
typedef void (*TaskFunction_t)(void *);

#define pdPASS ((BaseType_t)1)

#define configTICK_RATE_HZ 1000
#define portTICK_PERIOD_MS 1
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

void vTaskDelay(TickType_t ticks);

#ifdef __cplusplus
}
#endif
