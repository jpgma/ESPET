#include "freertos/FreeRTOS.h"

#include <windows.h>

void vTaskDelay(TickType_t ticks)
{
    DWORD ms = (DWORD)ticks * (DWORD)portTICK_PERIOD_MS;
    if (ms == 0) {
        ms = 1;
    }
    Sleep(ms);
}
