# Rasterizer plans

These tasks build the software rasterizer in this repo. Work them in order. Each file is a plan, not an implementation.

The stick-figure stage on the board repo uses this picture contract and does not redefine it: [vector index](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/vector/README.md).

These numbers are **this project’s choices**. Silicon law stays in [HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) and [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md). Do not copy the cadence, the pixel format, or the Wi-Fi policy into `HARDWARE.md`. Pins in this firmware come from `firmware/board_pins.h`.

## Locked choices

| Choice | Value |
| :--- | :--- |
| Scene | One creature plus a few props. A cap of **1024** triangles is the scratch budget. |
| Picture | Indexed-8 in one internal buffer, **57600** bytes (`240×240`). Palette of 256 RGB565 entries (**512** bytes). Flat color and vertex color. |
| Scanout | Two DMA bounce bands in internal DRAM. Start at **8** rows: `2 × 8 × 240 × 2 = 7680` bytes. Expand the palette only into a band. |
| Wire | Full frame at 80 MHz is **11.52 ms** (`240×240×16 / 80e6`). This unit measured **12271 µs** in [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md). |
| Present | No locked rate. Start the next frame as soon as this one finishes. The only wait is the glass ceiling, **80 Hz** (**12500 µs**). A slower frame runs at its own work time. A quiet frame skips SPI and does not sit out a slower tick. |
| Cores | Core 1 presents. Core 0 owns Wi-Fi, the I2C bus, audio, and the other simulations. |
| Handoff | Three pose slots. The sim publishes a timestamp and returns. Core 1 interpolates to the time the frame starts. A stalled sim holds the last pose. |
| Camera | Still on ordinary frames. A special animation marks all 240 rows. Pitch and roll may drive that camera later. Yaw is not a heading. |
| Occlusion | Painter’s order, far to near. |
| Radio | Wi-Fi up in the steady state, so the internal heap is the cortex case. Bluetooth off. The pet is still complete with the radio off. |
| Tearing | Accepted. This panel has no TE pin. |

## Order

| Task | Plan | Done when |
| ---: | :--- | :--- |
| 1 | [Budget page](./01-budget.md) | `budget.md` exists and the arithmetic matches this table |
| 2 | [Internal RAM with Wi-Fi](./02-internal-ram-wifi.md) | The indexed frame and both bands sit in internal DRAM, and a silicon log shows the heap that remains |
| 3 | [Present](./03-cadenced-present.md) | Dirty rows ship as soon as the frame is ready, and never faster than 80 Hz; an empty mask does not write GRAM |
| 4 | [One triangle](./04-one-triangle.md) | A known triangle matches a golden pixel list, walked in increasing Y |
| 5 | [Pose mailbox](./05-pose-mailbox.md) | A hitching producer does not slow present |
| 6 | [Mesh, clip, dirty rows](./06-mesh-clip-dirty.md) | A still scene produces an empty mask and no SPI |
| 7 | [Painter order](./07-painter.md) | The default scene has a written pixel-write count and a kept occlusion path |
| 8 | [Full-frame animation](./08-full-frame-cadence.md) | A scripted camera move presents at the rate the work allows, capped at the glass |

## Parked

Textures, a 120×120 render with 2× replicate, a 16-bit z-buffer, and SIMD fills stay parked unless a full frame in task 8 falls below the glass rate and that rate is unacceptable, or task 7 finds a real overlap that task 2 still has RAM for.

## Where the code goes

| Piece | Place | Includes |
| :--- | :--- | :--- |
| This project | `firmware/main/` | `main.c` may use the fake IDF headers. The filler, clip, and pose math are C11 and `stdint.h` only. No SDL. Silicon links `esp32/platform.c`. The sim links `win32/platform.c`. |
| New raster `.c` files | [`firmware/main/CMakeLists.txt`](../../firmware/main/CMakeLists.txt) `SRCS` and the `espet_firmware` sources in [`board-sim/CMakeLists.txt`](../../board-sim/CMakeLists.txt) | The next `sim.bat` rebuilds the sim list. `flash.bat` builds the silicon list. |

## Run

```text
sim.bat
```

The title bar shows raster, fps, frame period, slack against 12.5 ms (the 80 Hz glass cap), and the SPI clock. Click taps the fake CST816. This stub does not read that tap. Drag past a few pixels tilts the fake IMU.

## What you can prove where

`board-sim` fakes 240×240 GRAM and a polled QMI8658. `draw_bitmap` treats `x_end` / `y_end` as exclusive. `draw_bitmap` queues one SPI transfer from the firmware `pclk_hz` (`BOARD_SIM_SPI_HZ` overrides it). The next draw waits only if that transfer is still in flight, so a later delay or CPU stretch can cover the wire. That is one outstanding transfer, not GDMA. Host CPU work is stretched coarsely toward 240 MHz; the title is a rough landing, not the silicon pass bar.

The sim does not fake the second core, octal PSRAM latency, GDMA, or the Wi-Fi heap. Tasks 2, 5, and 8 say which half is host work and which half waits for the board.

Read next to these plans: [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md), [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md), [first pixels](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/first-pixels.md).
