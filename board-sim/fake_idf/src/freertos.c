#include "board_sim.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdlib.h>
#include <windows.h>

/* The CPU stretch clock belongs to app_main. Spawned tasks only sleep. */
static __declspec(thread) int s_spawned;

typedef struct {
    TaskFunction_t fn;
    void *arg;
} task_start_t;

static DWORD WINAPI task_thread(LPVOID p)
{
    task_start_t start = *(task_start_t *)p;
    free(p);
    s_spawned = 1;
    start.fn(start.arg);
    return 0;
}

BaseType_t xTaskCreate(TaskFunction_t task, const char *name,
                       const uint32_t stack, void *arg,
                       UBaseType_t priority, TaskHandle_t *out)
{
    (void)name;
    (void)stack;
    (void)priority;
    task_start_t *start = malloc(sizeof(*start));
    if (!task || !start) {
        free(start);
        return 0;
    }
    start->fn = task;
    start->arg = arg;
    HANDLE h = CreateThread(NULL, 0, task_thread, start, 0, NULL);
    if (!h) {
        free(start);
        return 0;
    }
    if (out) {
        *out = h;
    } else {
        CloseHandle(h);
    }
    return pdPASS;
}

void vTaskDelay(TickType_t ticks)
{
    DWORD ms = (DWORD)ticks * (DWORD)portTICK_PERIOD_MS;
    if (ms == 0) {
        ms = 1;
    }
    if (s_spawned) {
        Sleep(ms);
        return;
    }
    board_sim_cpu_charge();
    Sleep(ms);
    board_sim_cpu_yield();
}
