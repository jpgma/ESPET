#include "board_sim.h"
#include "freertos/FreeRTOS.h"

#include <windows.h>

void vTaskDelay(TickType_t ticks)
{
    board_sim_cpu_charge();
    DWORD ms = (DWORD)ticks * (DWORD)portTICK_PERIOD_MS;
    if (ms == 0) {
        ms = 1;
    }
    Sleep(ms);
    board_sim_cpu_yield();
}
