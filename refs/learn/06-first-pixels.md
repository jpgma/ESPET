# 06 — First pixels

← [05 quaternions](./05-quaternions.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 07 Indexed FB](./07-indexed-framebuffer.md) →

**Read:** [guide 03](../guides/03-display-st7789.md) · [ST7789V2.pdf](../display/ST7789V2.pdf) (commands: `0x2A` `CASET`, `0x2B` `RASET`, `0x2C` `RAMWR`, `0x3A` `COLMOD`) · [architecture 2 SPI note](../../architecture.md#2-hardware-map) · [firmware/main.c](../../firmware/main.c)

**Code:** you already have this file. This lesson is *understanding and cleaning*, not a rewrite.

## What you see in the sim

`sim.bat` builds `firmware/` against fake `esp_lcd`. `draw_bitmap` copies RGB565 into a 240×240 [GRAM](../glossary.md#gram). The window is that GRAM at 3× nearest-neighbor. Drag = fake IMU, not a camera.

## Walk `app_main`

1. **Backlight GPIO46** high. On silicon this should become [LEDC](../glossary.md#ledc) PWM (~30–40% on battery). High = studio / “I want to see something.”
2. **I2C bus** 41/42, device `0x6B`, `WHO_AM_I`. Lesson 08 owns the IMU; here you only need to know the screen still updates when the read works.
3. **`esp_lcd_new_panel_io_spi`** — CS 21, DC 45, mode 0, 40 MHz, [SPI3](../glossary.md#spi).
4. **`esp_lcd_new_panel_st7789`** — RST 40, 16 bpp, invert on (many IPS panels look solarized without it).
5. **Loop:** fill `s_fb` from accel tint + gyro bar, `draw_bitmap` full screen, delay 33 ms.

`rgb565()` packs 8-bit channels into 16 bits. The CPU writes every pixel every frame. That is the **wrong** long-term plan (115 KB × 2 plus radio is a bad bet) but it proves the pipe.

## `esp_lcd` vs owning the window

IDF does: bus, reset, init, `disp_on`, `draw_bitmap`.

You will own (lesson 07): dirty `CASET`/`RASET`, indexed expand, wait-previous-DMA.

No [TE](../glossary.md#te) pin: you cannot wait for vblank. 30 FPS is a software cap.

## File to keep

Stay in `firmware/main.c` until lesson 07 needs a second buffer. Do not add SDL. Do not raster a cube here.

## Checkpoint (sim)

`sim.bat`: window 720×720 (240×3). Drag changes the fill colour and the white bar. If the window is black, backlight/init path failed in the fake driver — check logs.

Optional cleanup: log SPI path once (`ESP_LOGI` already has WHO_AM_I). Add a solid red/green/blue fullscreen for one second each if colour order looks swapped (`LCD_RGB_ELEMENT_ORDER`).

## When the board arrives

Architecture §15 step 2:

- `BAT_EN` already high (lesson 13).
- Full-screen fill R/G/B.
- Print SPI microseconds at **40 vs 80 MHz**.
- If the image is scrambled with correct MADCTL, try SPI mode 3.
- Backlight PWM, not a static 1.

[Guide 03](../guides/03-display-st7789.md) bring-up test is this lesson on silicon.

← [05 quaternions](./05-quaternions.md) · [next: 07 Indexed FB](./07-indexed-framebuffer.md) →
