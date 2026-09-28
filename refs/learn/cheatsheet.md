# Cheat sheet

Keep this open while coding. Pins and schematic: [jpgma/esp32-s3 HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) and [board cheatsheet](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/cheatsheet.md). Course: [00 start](./00-start-here.md). Glossary: [glossary.md](../glossary.md).

## Golden rules

1. Core 1 never waits on Core 0, Wi-Fi, or the LLM.
2. The pet is complete with the radio off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. Sleeping the CPU is a feature. Idle I2S clocks or PA up is the same bug as a static 30 Hz SPI.
5. Meshes are appearance. The core spring is state. Bone clips are appearance. Pixels are not physics.
6. Clips write bone locals. The core spring only lags the body. Appendages do not spring.
7. The camera is room-authored and static until a clip sets `full_frame`. IMU tilt does not orbit.
8. Core 1 never plays audio. The sim on Core 0 renders one-shots into the play buffer. I2S DMA plays it.

## Pins (thin)

Full table is HARDWARE.md. Do not invent GPIOs.

| Function | GPIO | Notes |
| :--- | ---: | :--- |
| LCD SPI3 | 21 / 38 / 39, 45 / 40 / 46 | no TE, no MISO; 45/46 strap |
| I2C | 41 / 42 | **one owner task** on Core 0 |
| IMU INT | 6 | FIFO watermark (product). Hello still polls |
| Touch INT / RST | 48 / 47 | IRQ. `DisAutoSleep` |
| PA | 7 | ≥35 ms wake, ≥120 ms cold, ~80 ms shutdown |
| I2S | 8 / 9 / 10 / 11 / 12 | DIN unused |
| BAT_EN | 2 | first in `app_main`; hold high |
| VBAT / CHG | 1 / 3 | ×3; CHG active-low |
| PWR / PLUS / BOOT | 5 / 4 / 0 | |
| USB | 19 / 20 | native CDC. Dies in deep sleep |

GPIO18 is TF D1 — not a spare. TF unused. No RTC IC.

I2C: QMI8658 **0x6B**, CST816 **0x15**, ES8311 **0x18**, ES7210 **0x40 never probe**.

SPI: **mode 3, 80 MHz**, picture clear. GRAM 240×320, window 240×240. Full RGB565 ≈ **12.3 ms** measured → 30 FPS still wants dirty-rect, with ~21 ms left in the 33 ms budget.

Backlight LED typical **60 mA @ 3.0 V** — battery PWM **30–40%**. LEDC 5 kHz / 10-bit to start.

IMU start: **±8 g (4096 LSB/g)** / ±1024 dps.

## sdkconfig (this module)

```
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_SPEED_80M=y
# quad flash 80 MHz, not OPI
CONFIG_FREERTOS_HZ=1000
# Bluetooth off
# Wi-Fi only in cortex mode
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y
```

C++: `-fno-exceptions -fno-rtti`. IDF ≥ 5.5.

## Memory one-liners (ESPET)

- One indexed-8 FB in DRAM (57.6 KB). Two 8-row RGB565 DMA bands (7.7 KB). Triangle scratch ~32 KB.
- **Room palette** 256 (512 B), copy on door. Index 0 = key; 1–63 actor **ramps**; 64–255 room.
- Raster inner loop never touches PSRAM. Play buffer is DRAM (7.2 KB). No backdrop.
- Meshes and the weighted pet: flash XIP. Shadow/FX stamps: flash. Pose mailbox in DRAM (six bone 3×4s).
- Awake rigids ≤ 24. `FxPool` 256 stamps. Empty SPI mask when the pose matches.
- RTC: hunger/happy/sleep/emotion/`room_id`. 8 h is parked.

## Rates

| What | Rate |
| :--- | :--- |
| IMU + filter | 100 Hz |
| Sim (Core 0) and present (Core 1) | 30 Hz (33.3 ms deadline) |
| Play buffer | 12 kHz, 300 ms, DMA |
| Lizard (later) | 20 Hz |
| Housekeeping | 1–10 Hz |

## Sim vs silicon

```
sim.bat                 Windows fake board (this repo)
idf.py -C firmware …    real Waveshare (later)
```

`firmware/` has no SDL. `board-sim/` has no cube. Hardware PDFs: jpgma/esp32-s3.
