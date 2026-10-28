# Rasterizer plans

These tasks make the software rasterizer in this repository. Do the tasks in sequence. Each file is a plan. It is not an implementation.

The stick-figure stage in the board repository uses this picture contract. It does not change it: [vector index](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/vector/README.md).

These numbers are **the choices of this project**. The silicon rules stay in [HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) and [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md). Do not copy the cadence, the pixel format or the Wi-Fi policy into `HARDWARE.md`. The pins in this firmware come from `firmware/board_pins.h`.

## Locked choices

| Choice | Value |
| :--- | :--- |
| Scene | One creature and a small number of props. The scratch budget is a limit of **1024** triangles. |
| Picture | Indexed-8 in one internal buffer of **57600** bytes (`240×240`). A palette of 256 RGB565 entries (**512** bytes). Flat color and vertex color. |
| Scanout | Two DMA bounce bands in internal DRAM. Start with **8** rows: `2 × 8 × 240 × 2 = 7680` bytes. Expand the palette only into a band. |
| Wire | A full frame at 80 MHz is **11.52 ms** (`240×240×16 / 80e6`). This unit measured **12271 µs** in [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md). |
| Present | There is no locked rate. Start the next frame immediately when this frame is complete. The only wait is the glass limit, **80 Hz** (**12500 µs**). A slower frame runs at its own work time. A quiet frame does not use SPI, and it does not wait for a slower tick. |
| Cores | Core 1 presents. Core 0 owns Wi-Fi, the I2C bus, audio and the other simulations. |
| Handoff | Three pose slots. The sim publishes a timestamp and returns. Core 1 interpolates to the start time of the frame. If the sim stops, Core 1 holds the last pose. |
| Camera | The camera does not move on usual frames. A special animation marks all 240 rows. Later, pitch and roll can control that camera. Yaw is not a heading. |
| Occlusion | Painter's order, from far to near. |
| Radio | Wi-Fi is on in the steady state, so that the internal heap is the same as in the cortex case. Bluetooth is off. The pet is complete when the radio is off. |
| Tearing | Accepted. This panel has no TE pin. |

## Sequence

| Task | Plan | Complete when |
| ---: | :--- | :--- |
| 1 | [Budget page](./01-budget.md) | `budget.md` exists and its arithmetic agrees with this table |
| 2 | [Internal RAM with Wi-Fi](./02-internal-ram-wifi.md) | The indexed frame and the two bands are in internal DRAM, and a silicon log shows the remaining heap |
| 3 | [Present](./03-cadenced-present.md) | Dirty rows go out immediately when the frame is ready, and never faster than 80 Hz; an empty mask does not write GRAM |
| 4 | [One triangle](./04-one-triangle.md) | A known triangle agrees with a golden pixel list, written in increasing Y |
| 5 | [Pose mailbox](./05-pose-mailbox.md) | A producer that stops for short periods does not make the present slower |
| 6 | [Mesh, clip, dirty rows](./06-mesh-clip-dirty.md) | A still scene gives an empty mask and no SPI |
| 7 | [Painter order](./07-painter.md) | The default scene has a written pixel-write count and one occlusion path |
| 8 | [Full-frame animation](./08-full-frame-cadence.md) | A scripted camera move presents at the rate that the work permits, with the glass as the limit |

## Parked

These items stay parked: textures, a 120×120 render with 2× replication, a 16-bit z-buffer and SIMD fills. Use them only if one of these conditions occurs:

- In task 8, a full frame is slower than the glass rate, and that rate is not acceptable.
- Task 7 finds a real overlap, and task 2 shows that sufficient RAM is available for it.

## Code locations

| Part | Location | Includes |
| :--- | :--- | :--- |
| This project | `firmware/main/` | `main.c` can use the fake IDF headers. The filler, clip and pose math are C11 and use `stdint.h` only. No SDL. The silicon build links `esp32/platform.c`. The sim build links `win32/platform.c`. |
| New raster `.c` files | [`firmware/main/CMakeLists.txt`](../../firmware/main/CMakeLists.txt) `SRCS` and the `espet_firmware` sources in [`board-sim/CMakeLists.txt`](../../board-sim/CMakeLists.txt) | The next `sim.bat` builds the sim list again. `flash.bat` builds the silicon list. |

## Run

```text
sim.bat
```

The title bar shows the raster, the fps, the frame period, the slack against 12.5 ms (the 80 Hz glass limit) and the SPI clock. A click is a tap on the simulated CST816. This stub does not read that tap. A drag of more than a few pixels tilts the simulated IMU.

## What you can prove, and where

`board-sim` simulates a 240×240 GRAM and a polled QMI8658. For `draw_bitmap`, `x_end` and `y_end` are exclusive. `draw_bitmap` queues one SPI transfer at the firmware `pclk_hz` (`BOARD_SIM_SPI_HZ` replaces it). The next draw waits only if that transfer is not complete. Thus, a later delay or CPU work can occur at the same time as the wire. This is one queued transfer, not GDMA. The simulator stretches the host CPU work approximately to 240 MHz. The title gives an approximate value. It is not the pass criterion for silicon.

The simulator does not simulate the second core, the octal PSRAM latency, GDMA or the Wi-Fi heap. Tasks 2, 5 and 8 tell which part is host work and which part must wait for the board.

Read these documents with the plans: [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md), [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md), [first pixels](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/first-pixels.md).
