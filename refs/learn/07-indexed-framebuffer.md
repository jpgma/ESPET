# 07 — Indexed framebuffer and dirty rect

← [06 pixels](./06-first-pixels.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 08 IMU](./08-imu-registers.md) →

**Read:** [architecture 3. Memory](../../architecture.md#3-memory) · [architecture 10 dirty rect](../../architecture.md#10-rendering) · [guide 03](../guides/03-display-st7789.md) · [guide 02 memory law](../guides/02-soc-memory-smp.md)

**Code to produce:** one indexed-8 framebuffer in `firmware/` (still `main.c` is fine), a 256-entry [palette](../glossary.md#palette), expand dirty rows to RGB565 DMA [bands](../glossary.md#bounce-buffer), `draw_bitmap` only those rows.

## Why indexed-8

A 240×240 RGB565 frame is 115200 bytes. Two of them plus Wi-Fi DMA in internal RAM does not fit. One indexed-8 frame is 57600 bytes. Colour 0 = [key](../glossary.md#color-key) (transparent in stamps). The product palette is **256** entries: **1–63** actor ramps, **64–255** the room. This lesson may fill only a few slots.

**Scanout:** indexed frame → expand dirty rows into two 8-row RGB565 bands → [GDMA](../glossary.md#gdma) to ST7789.

The raster inner loop **never** touches [PSRAM](../glossary.md#psram). There is no photograph to restore.

## Palette

```c
uint16_t palette[256];          /* [0] is key; actor ramps live in 1..63 */
uint8_t  fb[240 * 240];         /* one frame, not two */
uint16_t band[2][8 * 240];      /* DMA bands */
```

Clear `fb` to a room index (not 0, or the glass looks empty). Draw the dummy block with an index in 1…63.

## Dirty rect

Do not SPI the whole glass. Union of “what changed” in pixel space: for now, the dummy sprite’s AABB + 2 px margin.

```
x0,y0,x1,y1   /* half-open or inclusive — match draw_bitmap */
```

ST7789: command `CASET` (`0x2A`) columns, `RASET` (`0x2B`) rows, `RAMWR` (`0x2C`) pixels. `esp_lcd_panel_draw_bitmap(panel, x0, y0, x1, y1, bounce_or_block)` is that window. Confirm whether `x1,y1` are exclusive (IDF usually exclusive end).

A 120×140 RGB565 rect ≈ 7 ms @ 40 MHz. Full frame ≈ 23 ms. Budget lives or dies here.

## GRAM-hold

If nothing moved, **do not SPI**. The panel [GRAM](../glossary.md#gram) keeps the last picture. Later: skip when the pose matches (architecture §4) — **not** `|Δq|`. For this lesson: skip `draw_bitmap` when the dummy block did not move.

Wait the **previous** DMA before kicking the next, not after physics in a way that stalls Core 0 (there is only one thread in sim; still structure it).

## Checkpoint (sim)

- Background index 1 (a dim colour), a moving block of index 2.
- Only a rectangle around the block should need to update (you can log rect size).
- When the block stops, SPI stops (log “hold”).
- Colour 0 in a sprite-shaped hole shows the background, not black unless background is 0.

## When the board arrives

Architecture §15 step 3: indexed FB + dirty dummy sprite. Print rect pixel count × 16 / spi_hz vs measured µs.

← [06 pixels](./06-first-pixels.md) · [next: 08 IMU](./08-imu-registers.md) →
