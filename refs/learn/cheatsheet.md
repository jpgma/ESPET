# Cheat sheet

Keep this page open while you write code. Pins and schematic: [jpgma/esp32-s3 HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) and [board cheatsheet](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/cheatsheet.md). Course: [00 start](./00-start-here.md). Glossary: [glossary.md](../glossary.md).

## Golden rules

1. Core 1 never waits for Core 0, Wi-Fi or the LLM.
2. The pet is complete when the radio is off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. A CPU that sleeps is correct. Idle I2S clocks or a PA that is on is the same bug as a still frame that sends SPI.
5. Meshes are appearance. The core spring is state. Bone clips are appearance. Pixels are not physics.
6. Clips write bone locals. The core spring only adds lag to the body. The appendages do not spring.
7. The room sets the camera. It stays static until a clip sets `full_frame`. IMU tilt does not turn the camera.
8. Core 1 never plays audio. The sim on Core 0 renders one-shots into the play buffer. I2S DMA plays it.

## Pins (short)

The full table is in HARDWARE.md. Do not invent GPIOs.

| Function | GPIO | Notes |
| :--- | ---: | :--- |
| LCD SPI3 | 21 / 38 / 39, 45 / 40 / 46 | No TE, no MISO; 45/46 are strapping pins |
| I2C | 41 / 42 | **One owner task** on Core 0 |
| IMU INT | 6 | FIFO watermark (product). Hello still polls. |
| Touch INT / RST | 48 / 47 | IRQ. `DisAutoSleep` |
| PA | 7 | Wake ≥35 ms, cold ≥120 ms, shutdown approximately 80 ms |
| I2S | 8 / 9 / 10 / 11 / 12 | DIN is not used |
| BAT_EN | 2 | First in `app_main`; hold high |
| VBAT / CHG | 1 / 3 | ×3; CHG is active-low |
| PWR / PLUS / BOOT | 5 / 4 / 0 | |
| USB | 19 / 20 | Native CDC. It stops in deep sleep. |

GPIO18 is TF D1. It is not a spare pin. The TF slot is not used. There is no RTC IC.

I2C: QMI8658 **0x6B**, CST816 **0x15**, ES8311 **0x18**, ES7210 **0x40 (never probe)**.

SPI: **mode 3, 80 MHz**, clear picture. GRAM is 240×320, the window is 240×240. A full RGB565 frame was measured at approximately **12.3 ms**. Dirty rows stay. A still pose sends nothing. The present limit is 80 Hz (12500 µs).

The backlight LED is usually **60 mA at 3.0 V**. On battery, use a PWM of **30–40%**. Start with LEDC 5 kHz / 10-bit.

IMU start: **±8 g (4096 LSB/g)** / ±1024 dps.

## sdkconfig (this module)

```
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_SPEED_80M=y
# quad flash 80 MHz, not OPI
CONFIG_FREERTOS_HZ=1000
# Bluetooth off
# Wi-Fi started (heap test is the cortex case)
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y
```

C++: `-fno-exceptions -fno-rtti`. IDF 5.5 or higher.

## Memory summary (ESPET)

- One indexed-8 FB in DRAM (57.6 KB). Two RGB565 DMA bands of 8 rows (7.7 KB). Triangle scratch for 1024 triangles (approximately 32 KB).
- **Room palette** of 256 (512 B). Copy it on a door. Index 0 = key; 1–63 actor **ramps**; 64–255 room.
- The raster inner loop never reads PSRAM. The play buffer is in DRAM (7.2 KB). No backdrop.
- Meshes and the weighted pet: flash XIP. Shadow and FX stamps: flash. Pose mailbox in DRAM (six bone 3×4 matrices).
- Awake rigids ≤ 24. `FxPool` of 256 stamps. An empty SPI mask when the pose is the same.
- RTC: hunger, happy, sleep, emotion, `room_id`. The 8 h target is parked.

## Rates

| Item | Rate |
| :--- | :--- |
| IMU and filter | 100 Hz |
| Sim (Core 0) | 30 Hz |
| Present (Core 1) | Glass limit 80 Hz (12500 µs); a slower frame runs at its work time |
| Play buffer | 12 kHz, 300 ms, DMA |
| Lizard brain (later) | 20 Hz |
| Housekeeping | 1–10 Hz |

## Simulator and silicon

```
sim.bat                 Windows fake board (this repo)
idf.py -C firmware …    real Waveshare (later)
```

`firmware/` has no SDL. `board-sim/` has no cube. The hardware PDFs are in jpgma/esp32-s3.
