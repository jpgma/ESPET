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
8. Core 1 never plays audio. The sim on Core 0 may emit `SfxEvt`. Core 0 mixes.

## Pins (thin)

Full table is HARDWARE.md. Do not invent GPIOs.

| Function | GPIO | Notes |
| :--- | ---: | :--- |
| LCD SPI3 | 21 / 38 / 39, 45 / 40 / 46 | no TE, no MISO; 45/46 strap |
| I2C | 41 / 42 | one bus, Core 0 owner |
| BAT_EN | 2 | hold high |
| USB | 19 / 20 | native CDC |

I2C: QMI8658 0x6B (hello), CST816 0x15, ES8311 0x18, ES7210 do not init.

SPI: try 80 MHz, fall back 40. Mode 0 first. Full 240×240 RGB565 @ 40 MHz ≈ 23 ms → 30 FPS + dirty rect.

## sdkconfig (this module)

```
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
- Mixer and raster never touch PSRAM. No backdrop.
- Meshes and the weighted pet: flash XIP. Shadow/FX stamps: flash. Pose mailbox in DRAM (six bone 3×4s).
- Awake rigids ≤ 24. `FxPool` 256 stamps. Empty SPI mask when the pose matches.
- RTC: hunger/happy/sleep/emotion/`room_id`. 8 h is parked.

## Rates

| What | Rate |
| :--- | :--- |
| IMU + filter | 100 Hz |
| Sim (Core 0) and present (Core 1) | 30 Hz (33.3 ms deadline) |
| Mixer | 12 kHz, 256-sample blocks |
| Lizard (later) | 20 Hz |
| Housekeeping | 1–10 Hz |

## Sim vs silicon

```
sim.bat                 Windows fake board (this repo)
idf.py -C firmware …    real Waveshare (later)
```

`firmware/` has no SDL. `board-sim/` has no cube. Hardware PDFs: jpgma/esp32-s3.
