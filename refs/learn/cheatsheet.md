# Cheat sheet

Keep this open while coding. Full pin story: [guide 01](../guides/01-board-and-pins.md). Course: [00 start](./00-start-here.md). Glossary: [glossary.md](../glossary.md).

## Golden rules

1. Core 1 never waits on Core 0, Wi-Fi, or the LLM.
2. The pet is complete with the radio off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. Sleeping the CPU is a feature. Idle I2S clocks or PA up is the same bug as a static 30 Hz SPI.
5. Sheets are appearance. Springs are state. Pixels are not physics. LOD is blit, not sim.
6. Animation and the matching sheet frame are the same pose. Springs only lag.
7. The camera is room-authored and static per room. Only yaw `view_idx` (0..3) is discrete.
8. Core 1 never plays audio. It may emit `SfxEvt`. Core 0 mixes.

## Pins (architecture §2)

Confirm once on [the schematic](../ESP32-S3-LCD-1.54-Schematic.pdf).

| Function | GPIO | Notes |
| :--- | ---: | :--- |
| LCD CS / CLK / MOSI | 21 / 38 / 39 | SPI3, write-only |
| LCD DC / RST / BL | 45 / 40 / 46 | **45, 46 strapping** |
| I2C SCL / SDA | 41 / 42 | One bus |
| Touch INT / RST | 48 / 47 | IRQ |
| IMU INT | 6 | FIFO watermark |
| PA enable | 7 | High only while a voice is live |
| I2S MCLK / BCLK / WS / DIN / DOUT | 8 / 9 / 10 / 11 / 12 | DIN unused v1 |
| BAT_EN | 2 | Hold high |
| VBAT ADC / CHG | 1 / 3 | Housekeeping |
| PWR / PLUS / BOOT | 5 / 4 / 0 | PLUS = lizard one-shot (not camera recenter) |
| USB D− / D+ | 19 / 20 | Native CDC / JTAG |

## I2C (Core 0 owner)

| Device | Addr | When |
| :--- | :--- | :--- |
| QMI8658 | 0x6A or **0x6B** (hello) | 100 Hz drain |
| CST816 | 0x15 | On INT |
| ES8311 | 0x18 | Play start/stop/volume only |
| ES7210 | — | Do not init |

## ST7789 commands you actually send

| Cmd | Hex | |
| :--- | :--- | :--- |
| SWRESET | `0x01` | After RST |
| SLPOUT | `0x11` | Wait ~120 ms |
| MADCTL | `0x36` | Rotation / RGB |
| COLMOD | `0x3A` | `0x55` = RGB565 |
| INVON | `0x21` | Often needed on IPS |
| DISPON | `0x29` | |
| CASET | `0x2A` | Dirty X |
| RASET | `0x2B` | Dirty Y |
| RAMWR | `0x2C` | Pixels (DMA) |

SPI: try 80 MHz, fall back to 40 MHz. Mode 0 first, confirm vs 3. No TE. No MISO.

Full 240×240 RGB565 @ 40 MHz ≈ 23 ms → 30 FPS + dirty rect.

## QMI8658 (A map — confirm in the PDF)

| Name | Addr | |
| :--- | :--- | :--- |
| WHO_AM_I | `0x00` | expect `0x05` here |
| CTRL1 | `0x02` | INT / iface |
| CTRL2 | `0x03` | accel ODR / FS |
| CTRL3 | `0x04` | gyro ODR / FS |
| CTRL7 | `0x08` | enable (`0x03` accel+gyro in hello) |
| CTRL9 | `0x0A` | host commands |
| FIFO_WTM_TH | `0x13` | watermark |
| FIFO_CTRL | `0x14` | mode |
| FIFO_SMPL_CNT | `0x15` | |
| FIFO_STATUS | `0x16` | overflow |
| FIFO_DATA | `0x17` | burst |
| AX_L | `0x35` | hello polls 12 bytes from here |

## CST816

| Name | Addr | |
| :--- | :--- | :--- |
| GestureID | `0x01` | `0x0B` = double-click = lizard one-shot |
| FingerNum | `0x02` | |
| XY | `0x03`–`0x06` | |
| ChipID | `0xA7` | log at boot |
| MotionMask | `0xEC` | EnDClick |
| IrqCtl | `0xFA` | |

## ES8311 (v1 DAC)

`0x00` reset · `0x01`–`0x03` clocks (ADC off, 12 kHz) · `0x09` SDP I2S 16-bit.

Standby between phrases. Full off only on deep sleep.

## sdkconfig

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

## Memory one-liners

- Indexed-8 FB in DRAM (2×57.6 KB). RGB565 bounce for SPI.
- Current **room palette** 32 (64 B), copy on door. Index 0 = key; 1–15 actors; 16–31 scenery.
- Mixer and blit never touch PSRAM. Backdrop restore may copy PSRAM → DRAM fb.
- Atlas: flash XIP, PSRAM if cache thrashes. Current room backdrop in PSRAM (57.6 KB).
- `FxPool` ~2 KB DRAM. GRAM-hold: springs/toys settled **and** `fx_live==0`.
- RTC: hunger/happy/sleep/emotion/`room_id`.

## Rates

| What | Rate |
| :--- | :--- |
| IMU + filter | 100 Hz |
| Display + physics | 30 Hz (33.3 ms) |
| Mixer | 12 kHz, 256-sample blocks |
| Lizard (later) | 20 Hz |
| Housekeeping | 1–10 Hz |

## Sim vs silicon

```
sim.bat                 Windows fake board
idf.py -C firmware …    real Waveshare (later)
```

`firmware/` has no SDL. `board-sim/` has no cube.
