# 07 — Indexed framebuffer and dirty rect

← [06 pixels](./06-first-pixels.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 08 IMU](./08-imu-registers.md) →

**Read:** [architecture 3. Memory](../../architecture.md#3-memory) · [architecture 10 dirty rect](../../architecture.md#10-rendering) · [guide 03](../guides/03-display-st7789.md) · [guide 02 memory law](../guides/02-soc-memory-smp.md)

**Code to produce:** indexed-8 back buffer in `firmware/` (still `main.c` is fine), 32-entry [palette](../glossary.md#palette), expand dirty rows to an RGB565 [bounce](../glossary.md#bounce-buffer), `draw_bitmap` only that window.

## Why indexed-8

A 240×240 RGB565 frame is 115200 bytes. Two of them plus Wi-Fi DMA in internal RAM does not fit the product. Indexed-8 is 57600 bytes per buffer. Colour 0 = [key](../glossary.md#color-key) (transparent in sprites). Palette has 32 real colours; a 256-slot table is still fine if slot 32…255 unused.

**Scanout:** indexed back buffer → expand dirty rows to RGB565 bounce → [GDMA](../glossary.md#gdma) to ST7789.

The blit inner loop **never** touches [PSRAM](../glossary.md#psram).

## Palette

```c
uint16_t palette[256];   /* fill 0..31 with rgb565(); [0] is key, never drawn as pet */
uint8_t  fb[240 * 240];  /* or two buffers: front/back */
uint16_t bounce[240];    /* one row, or a few rows */
```

Clear `fb` to 0. Draw with indices 1…31.

## Dirty rect

Do not SPI the whole glass. Union of “what changed” in pixel space: for now, the dummy sprite’s AABB + 2 px margin.

```
x0,y0,x1,y1   /* half-open or inclusive — match draw_bitmap */
```

ST7789: command `CASET` (`0x2A`) columns, `RASET` (`0x2B`) rows, `RAMWR` (`0x2C`) pixels. `esp_lcd_panel_draw_bitmap(panel, x0, y0, x1, y1, bounce_or_block)` is that window. Confirm whether `x1,y1` are exclusive (IDF usually exclusive end).

A 120×140 RGB565 rect ≈ 7 ms @ 40 MHz. Full frame ≈ 23 ms. Budget lives or dies here.

## GRAM-hold

If nothing moved, **do not SPI**. The panel [GRAM](../glossary.md#gram) keeps the last picture. Later the IMU always twitches: you will need `|Δq|` / `|Δcam|` deadband (lesson 10). For this lesson: skip `draw_bitmap` when the dummy sprite AABB did not change.

Wait the **previous** DMA before kicking the next, not after physics in a way that stalls Core 0 (there is only one thread in sim; still structure it).

## Checkpoint (sim)

- Background index 1 (a dim colour), a moving block of index 2.
- Only a rectangle around the block should need to update (you can log rect size).
- When the block stops, SPI stops (log “hold”).
- Colour 0 in a sprite-shaped hole shows the background, not black unless background is 0.

## When the board arrives

Architecture §15 step 3: indexed FB + dirty dummy sprite. Print rect pixel count × 16 / spi_hz vs measured µs.

← [06 pixels](./06-first-pixels.md) · [next: 08 IMU](./08-imu-registers.md) →
