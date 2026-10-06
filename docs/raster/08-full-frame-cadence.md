# 08 — Full-frame animation

← [painter order](./07-painter.md) · [index](./README.md)

**Goal:** a scripted camera move, which dirties every row, presents at the rate the work allows, capped at the glass. There is no slower locked period to fall back to.

**Depends on:** tasks 3, 5, 6, and 7.

**Read:** [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) 80 MHz measurement · [bring-up](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md) step 2

## The interval

For about two seconds, publish poses whose camera moves every sim tick and whose `full_frame` flag is set. The dummy publisher from task 5 keeps hitching on core 0 during the interval. Core 1 interpolates and presents.

Every frame of the interval:

- All 240 row bits are set.
- Work time is raster plus the wait for the last band’s DMA, measured with `esp_timer_get_time` or `CCOUNT` around that work, not around the idle wait for the glass cap.
- The next frame starts when that work finishes. It does not wait out another 12500 µs on top.

The steady scene, before and after the interval, still skips SPI when nothing moved.

## Pass bar

SPI is **80 MHz, mode 3**. Record every frame’s work time. The rate for the interval is the slower of the glass (**80 Hz**) and `1 / work_time`. The margin line in `budget.md` is the slack you thought you had inside 12500 µs; replace the guessed raster time with the measured one.

A frame slower than 12500 µs is not a missed deadline. Write the measured work time and the resulting rate. Do not add a sleep to hold a lower rate, and do not keep a second cadence for still frames.

## Host half

You can dry-run the flag on the sim. One SPI transfer stays in flight, so the next draw stalls only for whatever wire is still left. At 80 MHz a full frame is about 12 ms of wire. `BOARD_SIM_SPI_HZ` can override that clock. The title is a rough landing. A sim gap does **not** fail the task. The pass bar is the silicon measurement.

## Done when

- The interval presents as fast as the work allows, with the hitching publisher running, and never faster than 80 Hz.
- `budget.md` shows measured full-frame work time at 80 MHz, and the rate that work produced.
- Still frames outside the interval still produce an empty mask.

## After this

Textures, 120×120 with 2× replicate at scanout, and a SIMD filler are in scope only if the measured rate is unacceptable. Each of those is a new plan that starts by editing `budget.md`, not by adding a shader.
