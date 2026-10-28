# 01 — Budget page

← [index](./README.md) · [next: internal RAM](./02-internal-ram-wifi.md) →

**Objective:** make one page with the bytes, the wire time and the glass limit. You calculate these values. The subsequent tasks use them as targets.

**Prerequisites:** none.

**Read:** [index](./README.md) · [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md) · [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) · [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md)

## Output

Make [`budget.md`](./budget.md) in this folder. Fill all lines. Show the arithmetic, not only the result.

| Line | Procedure |
| :--- | :--- |
| Indexed frame | `240 × 240` bytes. One buffer. |
| Palette | `256 × 2` bytes. |
| Bounce | `2 × band_rows × 240 × 2`. Start with `band_rows = 8`. |
| Row mask | 240 bits. |
| Triangle scratch | The limit is **1024** triangles. Write the bytes of one screen-space triangle (positions, a color index, a sort key). Then multiply. |
| Pose mailbox | Three slots. Write the bytes of one pose (timestamp, camera, a small number of instance transforms). Then multiply. |
| Full-frame SPI | `240 × 240 × 16 / 80e6` seconds. The wire clock is 80 MHz. |
| Glass limit | `1 / 80` second, in microseconds. This is a maximum, not a target rate. |
| Margin | The glass limit minus the time to send a full frame at 80 MHz. This is the command and raster time that can occur with the wire before the rate becomes less than 80 Hz. A frame that uses more time runs slower. There is no second, slower period. |
| Core 1 cycles | `240e6 / 80`. This is the number of cycles on Core 1 in one glass period, with no other task on that core. A slower frame gets more cycles and a lower rate. |

Keep these lines empty until the related task measures them:

- The free internal heap when Wi-Fi is on and Bluetooth is off (task 2).
- The SPI microseconds for a full frame at 80 MHz on the glass (task 8). Guide 03 already measured **12271 µs** for a solid fill. Task 8 records the value for the raster output.
- The raster microseconds for the default scene and for a full-frame animation (tasks 7 and 8).

## Compare with the index

The index already gives the targets of the project: 57600, 512, 7680, 11.52 ms and 12500 µs. Your page must give the same values. If it does not, correct the arithmetic before you change the target.

## Complete when

- `budget.md` is in this folder, and all calculated lines are filled.
- The silicon lines are in the page, and they are still empty.
- You can show the margin line and tell its function.

## For later

Do not allocate the buffers yet. Do not select a z-buffer size. Task 2 is the first code.
