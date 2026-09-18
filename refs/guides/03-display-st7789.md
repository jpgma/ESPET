# 03 — Display ST7789 (240×240 SPI, no TE)

**Goal:** own `CASET`/`RASET`/`RAMWR`, DMA a dirty rect of RGB565, leave GRAM holding when the cube is still.

## PDFs

| File | Read |
| :--- | :--- |
| [`../display/ST7789V2.pdf`](../display/ST7789V2.pdf) | Command set, GRAM 240×320×18, SPI 4-wire, RGB565 `COLMOD`, MADCTL, partial/idle |
| [`../soc/esp32-s3_technical_reference_manual_en.pdf`](../soc/esp32-s3_technical_reference_manual_en.pdf) | SPI master + GDMA. Write-only MOSI |

IDF:

- [esp_lcd](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/lcd/index.html)
- [SPI master](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/spi_master.html)
- [LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/ledc.html) — backlight PWM on GPIO46

## What this panel is

- 1.54" IPS, **240×240** visible.
- Controller GRAM is **240×320**. The extra 80 rows exist. You set a window; you do not “fill the chip.”
- 4-wire SPI: CS, CLK, MOSI, DC. **No MISO. No TE.** You cannot vsync. 30 FPS is a software cap.
- Color on the wire is RGB565. ESPET keeps **indexed-8** in DRAM and expands only the dirty rows into a bounce buffer.

Pins: CS 21, CLK 38, MOSI 39, DC 45, RST 40, BL 46. Host **SPI3**. Try 80 MHz; fall back to 40 MHz.

Budget (`architecture.md` §2): full 240×240 RGB565 @ 40 MHz ≈ **23 ms**. That is why dirty-rect exists. A 120×140 rect ≈ 7 ms @ 40 MHz.

## Commands you will actually send

| Cmd | Hex | Use |
| :--- | :--- | :--- |
| Software reset | `0x01` | After RST pin |
| Sleep out | `0x11` | Then wait ~120 ms |
| MADCTL | `0x36` | MX/MY/MV/RGB. Fix rotation once on bring-up |
| COLMOD | `0x3A` | `0x55` = 16 bpp RGB565 |
| Inversion | `0x21` / `0x20` | Many IPS panels need INVON. If colors look solarized, flip this |
| Display on | `0x29` | |
| Column addr | `0x2A` `CASET` | Dirty rect X0–X1 (16-bit BE) |
| Row addr | `0x2B` `RASET` | Dirty rect Y0–Y1 |
| RAM write | `0x2C` `RAMWR` | Pixel burst. DC=1. DMA this |

Skip for v1: CABC, 18-bit RGB666, parallel 8080, tearing effect (`0x35` — **no pin**), RAM read.

SPI mode: datasheet + Waveshare cartoon say mode 0 (CPOL=0, CPHA=0). Architecture checklist still says **confirm 0 vs 3** on the glass. If the image is garbage with correct MADCTL, switch mode before rewriting the renderer.

## Driver shape (not LVGL)

Use `esp_lcd` for: bus init, panel IO, ST7789 reset + `esp_lcd_panel_init()`, `disp_on`.

Own:

1. Indexed back buffer in DRAM.
2. Dirty AABB (union of projected part AABBs + margin).
3. Expand those rows to RGB565 bounce (palette 32, color 0 = key already composited).
4. `CASET`/`RASET` for that window.
5. `esp_lcd_panel_draw_bitmap` / `tx_color` **without waiting** in the same place you kick physics — wait the **previous** DMA at the start of the next dirty frame (`architecture.md` §4).

GRAM-hold: if springs settled, no clip, toys settled, **`fx_live==0`**, **do not SPI**. The last frame stays on glass. Do **not** gate on `|Δq|` — the room camera is authored. A shake may dirty for ~0.3 s (dust FX), then hold. USB/studio may full-frame while FX live.

## Backlight

GPIO46, LEDC PWM. Battery default ~30–40%. USB = studio = brighter. Do not leave the pin as a static GPIO high at 100% on battery.

## What not to do

- IMU-orbit the room camera. Sheets are discrete yaws; the backdrop is static per room.
- Put the RGB565 FB in PSRAM and DMA it while Wi-Fi is up (architecture already rejected this).
- Use LVGL dirty-rect. You already have a backdrop + sprite renderer.

## Bring-up test (step 2–3)

Full-screen fill red/green/blue. Print SPI microseconds at 40 vs 80 MHz. Then an indexed dummy sprite with a moving dirty rect. Confirm GRAM holds when you stop updating (tilt must **not** restart SPI).
