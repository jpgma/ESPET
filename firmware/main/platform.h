/*
 * Chip and sim each supply one of these.
 * Sim: win32/platform.c. Board: esp32/platform.c.
 * DMA_ATTR comes from platform_target.h on that same include path.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_lcd_panel_io.h"
#include "platform_target.h"

/* On the chip this brings Wi-Fi up first, then logs where the buffers sit.
 * On the sim it prints the buffer sizes. */
void platform_prepare(const void *indexed, size_t indexed_bytes,
                      const void *palette, size_t palette_bytes,
                      const void *bands, size_t bands_bytes);

/* Arm the wait for a band that is on the wire.
 * The sim's draw_bitmap already holds the previous transfer. */
void platform_spi_arm(esp_lcd_panel_io_handle_t io);

void platform_wait_dma(void);
void platform_dma_queued(void);

/* Hold this frame's start until period_us after the previous start.
 * A late frame starts immediately. Missed ticks are not owed. */
void platform_pace(int64_t period_us);
