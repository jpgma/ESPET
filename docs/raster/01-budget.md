# 01 — Budget page

← [index](./README.md) · [next: internal RAM](./02-internal-ram-wifi.md) →

**Goal:** one page of bytes, wire time, and the glass cap, derived by you, so later tasks have a number to beat.

**Depends on:** nothing.

**Read:** [index](./README.md) · [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md) · [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) · [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md)

## What to produce

Create [`budget.md`](./budget.md) in this folder. Fill every line. Show the arithmetic, not only the result.

| Line | How to get it |
| :--- | :--- |
| Indexed frame | `240 × 240` bytes. One buffer. |
| Palette | `256 × 2` bytes. |
| Bounce | `2 × band_rows × 240 × 2`. Start at `band_rows = 8`. |
| Row mask | 240 bits. |
| Triangle scratch | Cap **1024** triangles. Write the bytes of one screen-space triangle (positions, a color index, a sort key) and multiply. |
| Pose mailbox | Three slots. Write the bytes of one pose (timestamp, camera, a handful of instance transforms) and multiply. |
| Full-frame SPI | `240 × 240 × 16 / 80e6` seconds. The wire clock is 80 MHz. |
| Glass cap | `1 / 80` second, in microseconds. A ceiling, not a target rate. |
| Margin | Glass cap minus the 80 MHz full-frame ship. This is how much command and raster time can sit beside the wire before the rate falls below 80 Hz. A frame that uses more time runs slower. There is no second, slower period. |
| Core 1 cycles | `240e6 / 80`. Cycles on core 1 inside one glass period, with no other task on it. A slower frame gets more cycles and a lower rate. |

Leave these blank until the matching task measures them:

- Free internal heap after Wi-Fi is up, Bluetooth off (task 2).
- SPI microseconds for a full frame at 80 MHz on the glass (task 8). Guide 03 already measured **12271 µs** for a solid fill; task 8 records the raster’s own ship.
- Raster microseconds for the default scene and for a full-frame animation (tasks 7 and 8).

## Check against the index

The index already states the project’s targets: 57600, 512, 7680, 11.52 ms, 12500 µs. Your page should land on those. If it does not, fix the arithmetic before changing the target.

## Done when

- `budget.md` is in this folder and every derived line is filled.
- The blank silicon lines are present and still blank.
- You can point at the margin line and say what it is for.

## Leave for later

Do not allocate the buffers yet. Do not pick a z-buffer size. Task 2 is the first code.
