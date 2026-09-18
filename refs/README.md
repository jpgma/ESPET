# Hardware study library (ESPET)

This folder is the reading set for programming the [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54) **as ESPET**, not as Waveshare’s LVGL/XiaoZhi demo.

**New to firmware?** Start at [`learn/00-start-here.md`](./learn/00-start-here.md). That course teaches C, buses, 3D math, and the build order against [`glossary.md`](./glossary.md). These guides stay blunt: which PDF page, which register.

Product lock lives in [`../architecture.md`](../architecture.md). These guides tell you **which pages of which PDF** matter for that lock, and what to ignore.

## Rules of use

1. **Schematic is law** after one pass: [`ESP32-S3-LCD-1.54-Schematic.pdf`](./ESP32-S3-LCD-1.54-Schematic.pdf) (same path `architecture.md` already cites).
2. **Steal Waveshare pins and register init, not the stack.** No LVGL, no Arduino, no 24 kHz duplex/AEC, no TF-card audio.
3. **ESP-IDF ≥ 5.5**, C++ as a better C (`-fno-exceptions -fno-rtti`).
4. Core 1 never waits. Core 0 owns I2C, IMU, mixer, optional Wi-Fi.

File list and original URLs: [`SOURCES.md`](./SOURCES.md).

## Curriculum (matches architecture §15)

Do these in order. Each guide names a PDF, the chapters, and a bring-up test.

| # | Guide | Hardware | Local PDFs |
| ---: | :--- | :--- | :--- |
| 1 | [Board and pins](./guides/01-board-and-pins.md) | Whole board, strapping, `BAT_EN` | schematic |
| 2 | [SoC, memory, SMP](./guides/02-soc-memory-smp.md) | ESP32-S3R8, octal PSRAM, quad flash, USB CDC | datasheet, TRM, errata, W25Q128 |
| 3 | [Display ST7789](./guides/03-display-st7789.md) | 240×240 SPI, dirty-rect, no TE | ST7789V2, TRM SPI/GDMA |
| 4 | [IMU QMI8658](./guides/04-imu-qmi8658.md) | I2C FIFO + INT | QMI8658A (primary), QMI8658C |
| 5 | [Fusion, gravity, IMU events](./guides/05-fusion-gravity-camera.md) | 6-axis complementary filter, sparse `imu_evt` | Madgwick ICORR 2011 |
| 6 | [Touch CST816](./guides/06-touch-cst816.md) | One finger, double-tap one-shot | CST816T registers + S English map |
| 7 | [Audio ES8311 + PA](./guides/07-audio-es8311.md) | 12 kHz TX, PA gated, ES7210 off | ES8311 user guide, NS4150B |
| 8 | [Power and sleep](./guides/08-power-battery.md) | `BAT_EN`, VBAT, PWR latch, GRAM-hold | ETA6098, sleep in IDF/TRM |
| 9 | [Bring-up checklist](./guides/09-bring-up.md) | Lab order vs §15 | all of the above |

## What you are building (one paragraph)

A gravity-locked habitat of cube rooms on this board. Core 0 runs QMI8658 at 100 Hz into a quaternion and sparse `imu_evt` (shake, not camera). Core 1, at 30 Hz, uses a **room-authored** 3/4 camera, picks one of **4 yaw sheets** (L2 parts in Nest/Play, L0 blob in Hall/Yard), restores a dirty window from a PSRAM backdrop, blits indexed-8 sprites, and DMA-s into ST7789 GRAM. Impacts enqueue `SfxEvt`; Core 0 mixes two procedural voices into ES8311 and raises the NS4150B only while a voice is live. Wi-Fi is a guest.

If a datasheet chapter does not serve that paragraph, skip it.
