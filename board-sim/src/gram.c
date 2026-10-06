#include "board_sim.h"

#include <string.h>
#include <windows.h>

static uint16_t s_gram[BOARD_SIM_LCD_W * BOARD_SIM_LCD_H];
static CRITICAL_SECTION s_gram_lock;
static int s_spi_hz; /* 0 = off */
static int s_spi_locked;
static uint8_t s_row_hit[BOARD_SIM_LCD_H];
static int s_rows_hit;
static LARGE_INTEGER s_qpc_freq;
static LONGLONG s_wire_until; /* one SPI transfer in flight; 0 = idle */

void board_sim_gram_init(void)
{
    InitializeCriticalSection(&s_gram_lock);
    memset(s_gram, 0, sizeof(s_gram));
    s_spi_hz = 0;
    s_wire_until = 0;
    QueryPerformanceFrequency(&s_qpc_freq);
}

static void sleep_until(LONGLONG until)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (s_qpc_freq.QuadPart <= 0 || now.QuadPart >= until) {
        return;
    }
    double remain_s = (double)(until - now.QuadPart) / (double)s_qpc_freq.QuadPart;
    if (remain_s >= 0.002) {
        DWORD ms = (DWORD)((remain_s - 0.001) * 1000.0);
        if (ms > 0) {
            Sleep(ms);
        }
    }
    do {
        QueryPerformanceCounter(&now);
    } while (now.QuadPart < until);
}

void board_sim_set_spi_hz(int hz)
{
    s_spi_hz = hz > 0 ? hz : 0;
    s_spi_locked = 1;
}

void board_sim_adopt_spi_hz(int hz)
{
    if (s_spi_locked || hz <= 0) {
        return;
    }
    s_spi_hz = hz;
}

int board_sim_spi_hz(void)
{
    return s_spi_hz;
}

void board_sim_spi_delay_pixels(size_t pixel_count)
{
    /* Stretch the CPU work, then stall only for wire still in flight.
     * This transfer is queued and overlaps the next delay or compute. */
    board_sim_cpu_charge();
    if (s_qpc_freq.QuadPart <= 0) {
        QueryPerformanceFrequency(&s_qpc_freq);
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (s_wire_until > now.QuadPart) {
        sleep_until(s_wire_until);
        QueryPerformanceCounter(&now);
    }
    if (s_spi_hz > 0 && pixel_count > 0 && s_qpc_freq.QuadPart > 0) {
        double seconds = (double)pixel_count * 16.0 / (double)s_spi_hz;
        s_wire_until = now.QuadPart + (LONGLONG)(seconds * (double)s_qpc_freq.QuadPart + 0.5);
    } else {
        s_wire_until = now.QuadPart;
    }
    board_sim_cpu_yield();
}

int board_sim_gram_blit(int x0, int y0, int x1, int y1, const uint16_t *rgb565)
{
    if (!rgb565 || x1 <= x0 || y1 <= y0) {
        return 0;
    }
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 > BOARD_SIM_LCD_W) {
        x1 = BOARD_SIM_LCD_W;
    }
    if (y1 > BOARD_SIM_LCD_H) {
        y1 = BOARD_SIM_LCD_H;
    }

    const int w = x1 - x0;
    const int h = y1 - y0;
    if (w <= 0 || h <= 0) {
        return 0;
    }
    EnterCriticalSection(&s_gram_lock);
    for (int y = 0; y < h; y++) {
        memcpy(&s_gram[(y0 + y) * BOARD_SIM_LCD_W + x0],
               &rgb565[y * w],
               (size_t)w * sizeof(uint16_t));
    }
    for (int y = y0; y < y1; y++) {
        if (!s_row_hit[y]) {
            s_row_hit[y] = 1;
            s_rows_hit++;
        }
    }
    int closed = 0;
    if (s_rows_hit >= BOARD_SIM_LCD_H) {
        closed = 1;
        memset(s_row_hit, 0, sizeof(s_row_hit));
        s_rows_hit = 0;
    }
    LeaveCriticalSection(&s_gram_lock);
    return closed;
}

void board_sim_gram_copy(uint16_t *dst)
{
    EnterCriticalSection(&s_gram_lock);
    memcpy(dst, s_gram, sizeof(s_gram));
    LeaveCriticalSection(&s_gram_lock);
}
