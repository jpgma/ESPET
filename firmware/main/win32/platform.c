#include "platform.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static LARGE_INTEGER s_qpc_freq;
static int64_t s_last_frame_start_us;
static int s_has_started;

static int64_t host_time_us(void)
{
    if (s_qpc_freq.QuadPart == 0) {
        if (!QueryPerformanceFrequency(&s_qpc_freq) || s_qpc_freq.QuadPart <= 0) {
            s_qpc_freq.QuadPart = 0;
            return 0;
        }
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    int64_t freq = s_qpc_freq.QuadPart;
    int64_t ticks = now.QuadPart;
    return (ticks / freq) * 1000000LL + ((ticks % freq) * 1000000LL) / freq;
}

void platform_prepare(const void *indexed, size_t indexed_bytes,
                      const void *palette, size_t palette_bytes,
                      const void *bands, size_t bands_bytes)
{
    (void)indexed;
    (void)palette;
    (void)bands;
    printf("indexed %u palette %u bands %u\n",
           (unsigned)indexed_bytes,
           (unsigned)palette_bytes,
           (unsigned)bands_bytes);
}

void platform_spi_arm(esp_lcd_panel_io_handle_t io)
{
    (void)io;
}

void platform_wait_dma(void)
{
}

void platform_dma_queued(void)
{
}

void platform_pace(int64_t period_us)
{
    int64_t now = host_time_us();
    if (s_qpc_freq.QuadPart == 0) {
        s_has_started = 1;
        return;
    }
    int64_t next = s_last_frame_start_us + period_us;
    if (s_has_started && now < next) {
        int64_t remain_us = next - now;
        if (remain_us > 2000) {
            Sleep((DWORD)((remain_us - 1000) / 1000));
        }
        do {
            now = host_time_us();
        } while (now < next);
    }
    s_last_frame_start_us = now;
    s_has_started = 1;
}
