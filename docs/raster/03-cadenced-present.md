# 03 — Present

← [internal RAM](./02-internal-ram-wifi.md) · [index](./README.md) · [next: one triangle](./04-one-triangle.md) →

**Objective:** send the dirty rows to GRAM immediately when the frame is ready, but never faster than the glass. An empty mask must not start SPI.

**Prerequisites:** the buffers from task 2. The silicon heap log can still be empty, because the present path has a host part.

**Read:** [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) · [first pixels](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/first-pixels.md) · `esp_lcd_panel_draw_bitmap` in [`esp_lcd.h`](../../board-sim/fake_idf/include/esp_lcd.h)

## Clock

There is no target frame rate. Core 1 starts the next frame immediately when the previous frame is complete. The only wait is the glass limit: **80 Hz**, **12500 µs** between the starts of two frames.

Keep the next start on an absolute clock (`esp_timer`). If this frame is complete early, wait until `last_start + 12500`. If it is complete late, start the next frame immediately. Then calculate the subsequent start from that new time. Do not add a delay after the work. Do not keep a count of missed ticks. A delay after the work adds jitter. A count of missed ticks would skip presents to get a rate that is not the target.

Do not start a second frame while the first frame is still in transfer.

The simulator has no `esp_timer` and no second core. Run the same loop from a host clock, and record the gap between the starts. The fake `vTaskDelay` is a Windows `Sleep`. Do not use it for this limit.

`draw_bitmap` queues one SPI transfer at `pclk_hz` (`BOARD_SIM_SPI_HZ` replaces it) and returns. The next draw waits only while that transfer is not complete. Thus, the subsequent wait or CPU work occurs at the same time as the wire. This is one queued transfer, not GDMA. The CPU stretch to 240 MHz is not Xtensa timing. The period in the title is an approximate value. The pass criterion is the silicon measurement.

## Steps for one frame

1. Get the frame start. If the glass limit has not elapsed, wait here.
2. Read the row mask. It has **240** bits, one bit for each row.
3. If no bit is set, go back to the wait. Do not call `draw_bitmap`. GRAM keeps its contents.
4. Go through the set rows from top to bottom, in bands of `band_rows` (8).
5. Expand those indexed bytes through the palette into the idle bounce buffer. Use RGB565 in the native byte order that the panel driver already uses.
6. Wait until the DMA of the previous band is complete. Then call `draw_bitmap` for that band. `x_end` and `y_end` are exclusive.
7. After the last band, clear the mask.

The indexed buffer is not the DMA source. After the bytes of a band are in the bounce buffer, the filler can write those rows again. When the next frame clears a row, it must wait until the DMA of that row is complete. With one indexed frame, use this simple rule: do not clear the indexed buffer until the DMA of the last band of this frame is complete.

There are two bounce buffers, so that the expansion of band *k+1* can occur while band *k* is on the wire. On the simulator, `draw_bitmap` copies into GRAM and queues the wire. The wait for the previous transfer occurs at the start of the next `draw_bitmap`. On the chip, that wait is the DMA-done check. It is not an I2C path.

Replace the marker loop in `firmware/main/main.c` with this path. Keep the panel initialization. For this task, the picture is a solid indexed clear and a rectangle that you fill manually. `sim.bat` is the window.

## Tests

Host and simulator:

- Fill the indexed buffer with palette index A. Mark rows 8–23. Present one time. The window shows a band, and the remaining area keeps the previous GRAM color.
- On the next tick, clear the mask and present. The band does not change. If you write index B and B shows, the empty mask still called SPI.
- Record ten start gaps for the 16-row band. They are near 12.5 ms. The band takes less time than the glass, thus the limit sets the gap. A Windows sleep is not accurate. Record the error. Keep the limit as an absolute time, so that the silicon path does not use "delay after work".
- Record ten start gaps for a full frame. They follow the work: approximately 12.3 ms on this glass. They do not add another 12.5 ms.

Silicon, later:

- Use the same two frames. Print the microseconds that the transfer takes at 80 MHz for the 16 rows, and for all 240 rows.
- SPI on this glass is **mode 3 at 80 MHz** ([guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md)). A full frame was measured at **12271 µs**.

## Complete when

- An empty mask does not call `draw_bitmap`.
- A partial mask updates only those rows.
- A fast frame starts on the 12.5 ms glass limit. A frame that is slower than the limit starts immediately when the previous transfer is complete.
- The present function goes back to the wait. It does not use the I2C bus.

## For later

No triangles, no pose interpolation. Task 8 measures the full-frame work.
