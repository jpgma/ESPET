#include "board_sim.h"

#include <string.h>
#include <windows.h>

static uint16_t s_gram[BOARD_SIM_LCD_W * BOARD_SIM_LCD_H];
static CRITICAL_SECTION s_gram_lock;
static int s_spi_hz; /* 0 = off */

void board_sim_gram_init(void)
{
    InitializeCriticalSection(&s_gram_lock);
    memset(s_gram, 0, sizeof(s_gram));
    s_spi_hz = 0;
}

void board_sim_set_spi_hz(int hz)
{
    s_spi_hz = hz > 0 ? hz : 0;
}

int board_sim_spi_hz(void)
{
    return s_spi_hz;
}

void board_sim_spi_delay_pixels(size_t pixel_count)
{
    if (s_spi_hz <= 0 || pixel_count == 0) {
        return;
    }
    /* RGB565: 16 bits per pixel, 4-wire SPI, no TE. */
    double seconds = (double)pixel_count * 16.0 / (double)s_spi_hz;
    DWORD ms = (DWORD)(seconds * 1000.0 + 0.5);
    if (ms < 1 && seconds > 0.0) {
        ms = 1;
    }
    if (ms > 0) {
        Sleep(ms);
    }
}

void board_sim_gram_blit(int x0, int y0, int x1, int y1, const uint16_t *rgb565)
{
    if (!rgb565 || x1 <= x0 || y1 <= y0) {
        return;
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
    EnterCriticalSection(&s_gram_lock);
    for (int y = 0; y < h; y++) {
        memcpy(&s_gram[(y0 + y) * BOARD_SIM_LCD_W + x0],
               &rgb565[y * w],
               (size_t)w * sizeof(uint16_t));
    }
    LeaveCriticalSection(&s_gram_lock);
}

void board_sim_gram_copy(uint16_t *dst)
{
    EnterCriticalSection(&s_gram_lock);
    memcpy(dst, s_gram, sizeof(s_gram));
    LeaveCriticalSection(&s_gram_lock);
}
