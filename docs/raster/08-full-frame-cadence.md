# 08 — Full-frame animation

← [painter order](./07-painter.md) · [index](./README.md)

**Objective:** a scripted camera move makes all rows dirty. It must present at the rate that the work permits, with the glass as the limit. There is no slower locked period to use as a fallback.

**Prerequisites:** tasks 3, 5, 6 and 7.

**Read:** [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) (80 MHz measurement) · [bring-up](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md) step 2

## The interval

For approximately two seconds, publish poses that have the `full_frame` flag set and a camera that moves on each sim tick. During the interval, the dummy publisher from task 5 continues to stop for short periods on Core 0. Core 1 interpolates and presents.

On each frame of the interval:

- All 240 row bits are set.
- The work time is the raster time plus the wait for the DMA of the last band. Measure it with `esp_timer_get_time` or `CCOUNT` around that work. Do not include the idle wait for the glass limit.
- The next frame starts when that work is complete. It does not wait another 12500 µs.

Before and after the interval, the steady scene still does not use SPI when nothing moves.

## Pass criterion

SPI is **80 MHz, mode 3**. Record the work time of each frame. The rate for the interval is the slower of the glass (**80 Hz**) and `1 / work_time`. The margin line in `budget.md` is the slack that you expected in 12500 µs. Replace the estimated raster time with the measured value.

A frame that is slower than 12500 µs is not a missed deadline. Write the measured work time and the rate that it gives. Do not add a sleep to keep a lower rate. Do not keep a second cadence for still frames.

## Host part

You can test the flag on the simulator. One SPI transfer stays in the queue. Thus, the next draw waits only for the remaining wire time. At 80 MHz, a full frame is approximately 12 ms of wire time. `BOARD_SIM_SPI_HZ` can replace that clock. The title gives an approximate value. A gap in the simulator does **not** make the task fail. The pass criterion is the silicon measurement.

## Complete when

- The interval presents as fast as the work permits, with the publisher that stops for short periods, and never faster than 80 Hz.
- `budget.md` shows the measured full-frame work time at 80 MHz, and the rate that this work gave.
- Still frames outside the interval still give an empty mask.

## After this task

These items are in scope only if the measured rate is not acceptable: textures, 120×120 with 2× replication at scanout, and a SIMD filler. Each of these items is a new plan. That plan starts with a change to `budget.md`, not with a new shader.
