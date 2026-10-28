# 07 — Indexed framebuffer and dirty rect

← [06 pixels](./06-first-pixels.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 08 IMU](./08-imu-registers.md) →

**Read:** [architecture 3. Memory](../../architecture.md#3-memory) · [architecture 10 dirty rect](../../architecture.md#10-rendering) · [guide 03](../guides/03-display-st7789.md) · [guide 02 memory rules](../guides/02-soc-memory-smp.md)

**Code to make:** one indexed-8 framebuffer in `firmware/` (`main.c` is satisfactory for now) and a [palette](../glossary.md#palette) of 256 entries. Expand the dirty rows to RGB565 DMA [bands](../glossary.md#bounce-buffer). Call `draw_bitmap` only for those rows.

## Why indexed-8

A 240×240 RGB565 frame is 115200 bytes. Two of these frames and Wi-Fi DMA do not fit in internal RAM. One indexed-8 frame is 57600 bytes. Color 0 is the [key](../glossary.md#color-key) (transparent in stamps). The product palette has **256** entries: **1–63** for the actor ramps and **64–255** for the room. In this lesson, you can fill only a small number of entries.

**Scanout:** indexed frame → expand the dirty rows into two RGB565 bands of 8 rows → [GDMA](../glossary.md#gdma) to the ST7789.

The raster inner loop **never** reads [PSRAM](../glossary.md#psram). There is no photograph to restore.

## Palette

```c
uint16_t palette[256];          /* [0] is key; actor ramps live in 1..63 */
uint8_t  fb[240 * 240];         /* one frame, not two */
uint16_t band[2][8 * 240];      /* DMA bands */
```

Clear `fb` to a room index. (Do not use 0, or the glass looks empty.) Draw the dummy block with an index in 1…63.

## Dirty rect

Do not send all of the glass through SPI. Use the union of the changes in pixel space. For now, this is the AABB of the dummy sprite with a margin of 2 px.

```
x0,y0,x1,y1   /* half-open or inclusive — match draw_bitmap */
```

ST7789 commands: `CASET` (`0x2A`) for the columns, `RASET` (`0x2B`) for the rows, `RAMWR` (`0x2C`) for the pixels. `esp_lcd_panel_draw_bitmap(panel, x0, y0, x1, y1, bounce_or_block)` sends that window. Make sure whether `x1,y1` are exclusive. (In IDF, the end is usually exclusive.)

A 120×140 RGB565 rect takes approximately 7 ms at 40 MHz. A full frame takes approximately 23 ms. The budget depends on this.

## GRAM hold

If nothing moved, **do not use SPI**. The panel [GRAM](../glossary.md#gram) keeps the last picture. Later, skip SPI when the pose is the same (architecture §4). Do **not** use `|Δq|`. For this lesson: do not call `draw_bitmap` when the dummy block did not move.

Wait for the **previous** DMA before you start the next one. Do not wait after the physics in a way that stops Core 0. (The simulator has only one thread, but use this structure.)

## Checkpoint (simulator)

- The background is index 1 (a dark color), and a moving block is index 2.
- Only a rectangle around the block must update. You can record the size of the rect.
- When the block stops, SPI stops. Record "hold".
- Color 0 in a hole with the shape of a sprite shows the background. It is black only if the background is 0.

## When the board arrives

Architecture §15 step 3: indexed FB and a dirty dummy sprite. Print the rect pixel count × 16 / spi_hz, and compare it with the measured µs.

← [06 pixels](./06-first-pixels.md) · [next: 08 IMU](./08-imu-registers.md) →
