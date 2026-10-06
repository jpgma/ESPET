# 03 — Present

← [internal RAM](./02-internal-ram-wifi.md) · [index](./README.md) · [next: one triangle](./04-one-triangle.md) →

**Goal:** dirty rows reach GRAM as soon as the frame is ready, never faster than the glass, and an empty mask does not start SPI.

**Depends on:** task 2’s buffers. The silicon heap log can still be blank; the present path has a host half.

**Read:** [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) · [first pixels](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/first-pixels.md) · `esp_lcd_panel_draw_bitmap` in [`esp_lcd.h`](../../board-sim/fake_idf/include/esp_lcd.h)

## Clock

There is no target frame rate. Core 1 starts the next frame as soon as the previous one has finished. The only wait is the glass ceiling: **80 Hz**, **12500 µs** between frame starts.

Keep the next start on an absolute clock (`esp_timer`). If this frame finished early, wait until `last_start + 12500`. If it finished late, start the next one immediately and set the following start from that new time. Do not add a delay after the work, and do not keep a debt of missed ticks. A delay after the work stacks jitter. A debt would skip presents to catch a rate you are not trying to hold.

Do not start a second frame on top of one that is still shipping.

On the sim there is no `esp_timer` and no second core. Drive the same loop from a host clock and log the start-to-start gap. The fake `vTaskDelay` is a Windows `Sleep` and is the wrong tool for this cap.

`draw_bitmap` queues one SPI transfer from `pclk_hz` (`BOARD_SIM_SPI_HZ` overrides it) and returns. The next draw waits only while that transfer is still in flight, so the following wait or CPU work overlaps the wire. That is one outstanding transfer, not GDMA, and the CPU stretch toward 240 MHz is not Xtensa timing. The title period is a rough landing. The pass bar is still the silicon measurement.

## What one frame does

1. Take the frame start. If the glass cap has not elapsed, this is where the wait sits.
2. Read the row mask. **240** bits, one bit per row.
3. If no bit is set, return to the wait. Do not call `draw_bitmap`. GRAM holds.
4. Walk set rows from top to bottom in bands of `band_rows` (8).
5. Expand those indexed bytes through the palette into the idle bounce. RGB565, native endianness the panel driver already uses.
6. Wait until the previous band’s DMA has finished, then `draw_bitmap` that band. `x_end` / `y_end` are exclusive.
7. After the last band, clear the mask.

The indexed buffer is not the DMA source. Once a band’s bytes have been copied into the bounce, the filler may later overwrite those rows. The next frame’s clear of a row waits until that row’s DMA has finished. With one indexed frame, the easy rule is: do not clear the indexed buffer until the last band of this frame has finished DMA.

Two bounces exist so the expand of band *k+1* can run while band *k* is on the wire. On the sim, `draw_bitmap` copies into GRAM and queues the wire; the wait for the previous transfer sits at the start of the next `draw_bitmap`. On the chip, that wait is the DMA done check, not an I2C path.

Replace the marker loop in `firmware/main/main.c` with this path. Keep panel init. A solid indexed clear plus a hand-filled rectangle is the picture for this task. `sim.bat` is the window.

## Tests

Host / sim:

- Fill the indexed buffer with palette index A, mark rows 8–23, present once. The window shows a band and the rest stays at the previous GRAM color.
- Next tick, clear the mask and present. The band does not change. If you would have written index B, and B appears, the empty mask still called SPI.
- Log ten start gaps for the 16-row band. They cluster on 12.5 ms. The band is cheaper than the glass, so the cap is what spaces them. A Windows sleep will wander; record the wander, and keep the source of the cap as an absolute time so the silicon path does not inherit “delay after work.”
- Log ten start gaps for a full-frame ship. They follow the work, about 12.3 ms on this glass, and they do not add another 12.5 ms on top.

Silicon, later:

- Same two frames. Print microseconds spent inside the ship at 80 MHz for those 16 rows, and for all 240 rows.
- SPI on this glass is **mode 3 at 80 MHz** ([guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md)). A full frame measured **12271 µs**.

## Done when

- An empty mask does not call `draw_bitmap`.
- A partial mask updates only those rows.
- A cheap frame starts on the 12.5 ms glass cap. A frame slower than that starts as soon as the previous ship finishes.
- The present function returns to the wait without taking the I2C bus.

## Leave for later

No triangles, no pose interpolation. The full-frame work measurement is task 8.
