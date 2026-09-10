# 02 — How chips talk

← [01 C](01-c-for-firmware.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 03 Tasks](03-tasks-cores-timing.md) →

**Read:** [guide 01](../guides/01-board-and-pins.md) · [architecture 2. Hardware map](../../architecture.md#2-hardware-map) · schematic [ESP32-S3-LCD-1.54-Schematic.pdf](../ESP32-S3-LCD-1.54-Schematic.pdf) (skim the net names only)

The ESP32-S3 is the brain. The LCD, IMU, touch, and codec are **other chips** with their own tiny computers or state machines. You talk to them over wires.

## Registers as mailboxes

A [register](../glossary.md#register) is a named byte (or word) inside a chip. You write `0x03` to QMI8658 `CTRL7` (`0x08`) and accel+gyro turn on. You read `WHO_AM_I` (`0x00`) and get `0x05` if the chip is alive.

A datasheet **register map** is a table: address, name, bits. Each bit field is a switch. You do not need to understand the analog guts.

[Memory-mapped I/O](../glossary.md#mmio) is the same idea *inside* the ESP32: SPI, GPIO, I2S look like RAM addresses. ESP-IDF hides most of that. You still think “write this config struct, then the peripheral runs.”

## GPIO

A [GPIO](../glossary.md#gpio) pin is a wire you drive high (3.3 V) or low (0 V), or read as input.

On this board:

- GPIO46 = backlight (output)
- GPIO7 = [PA](../glossary.md#pa) enable
- GPIO2 = `BAT_EN` (must stay high on battery)
- GPIO6 = IMU [INT](../glossary.md#irq)
- GPIO48 = touch INT

`gpio_config` + `gpio_set_level` in `main.c` turns the backlight on. That is not PWM yet ([LEDC](../glossary.md#ledc) in lesson 06 / 13).

**[Strapping pins](../glossary.md#strapping-pin):** GPIO0, 45, 46 mean something *at reset* (boot mode, voltages). After boot they are ordinary pins. LCD DC is 45, backlight is 46. Do not fight download mode; after `app_main` they are yours.

## I2C

[I2C](../glossary.md#i2c) is two wires: [SCL](../glossary.md#scl) (clock) and [SDA](../glossary.md#sda) (data). Many slaves, one master.

Each slave has a 7-bit [address](../glossary.md#i2c-address). The master says “I want 0x6B” and the chip [ACKs](../glossary.md#ack). Then a register address, then data.

ESPET **one bus**: SCL 41, SDA 42.

| Chip | Addr | Role |
| :--- | :--- | :--- |
| [QMI8658](../glossary.md#qmi8658) | 0x6A or 0x6B | IMU |
| [CST816](../glossary.md#cst816) | 0x15 | touch |
| [ES8311](../glossary.md#es8311) | 0x18 | codec |
| [ES7210](../glossary.md#es7210) | — | **do not init** |

Write-then-read (what `i2c_master_transmit_receive` does): send register number, then read N bytes. The IMU burst in `main.c` starts at `AX_L` and pulls 12 bytes (accel xyz + gyro xyz).

**One software owner** (Core 0). If the 100 Hz IMU drain and a codec init share the bus without a single owner, the filter stalls. Architecture forbids codec writes inside the IMU task.

## SPI

[SPI](../glossary.md#spi) is faster, usually one slave per chip-select.

Wires here: [CS](../glossary.md#cs) 21, [CLK](../glossary.md#clk) 38, [MOSI](../glossary.md#mosi) 39, [DC](../glossary.md#dc) 45. **No [MISO](../glossary.md#miso).** The panel is write-only.

[DC](../glossary.md#dc) = 0 means “this byte is a command” (`CASET`). DC = 1 means “this is data” (pixels). That extra pin is why LCD SPI is not “classic 4-wire with MISO.”

[SPI mode](../glossary.md#spi-mode) 0 vs 3 is clock idle level and which edge samples. Architecture checklist: confirm on glass. `main.c` uses mode 0.

Speed: try 80 MHz, fall back to 40 MHz. A full 240×240 RGB565 frame at 40 MHz is ~23 ms. That is why 30 [FPS](../glossary.md#fps) and dirty rects exist.

## I2S

[I2S](../glossary.md#i2s) is for audio samples, not commands. Clocks: [MCLK](../glossary.md#mclk) 8, [BCLK](../glossary.md#bclk) 9, [WS](../glossary.md#ws) 10. Data: DOUT 12 to the codec, DIN 11 unused in v1.

You will not code this until lesson 15. Know it is a *stream*, gated with the PA, not a file on the TF card.

## DMA

[DMA](../glossary.md#dma) / [GDMA](../glossary.md#gdma): the SPI (or I2S) engine copies bytes from RAM while the CPU does something else. Core 1 kicks a dirty rect, then later waits for the **previous** transfer — never for Core 0.

DMA cannot always see [PSRAM](../glossary.md#psram) the way the CPU cache does. Framebuffers stay in [DRAM](../glossary.md#dram).

## Exercise

On the [cheat sheet](cheatsheet.md), highlight: which pins are I2C, which are SPI, which are “just GPIO.” Check the schematic net names match architecture §2. If they disagree, schematic wins.

## Checkpoint

You can explain, without looking, why four chips share 41/42 and why the LCD has no MISO.

## When the board arrives

[Guide 01](../guides/01-board-and-pins.md) bring-up: `BAT_EN` high, USB prints hello. That is lesson 13. Today you only need the map.

← [01 C](01-c-for-firmware.md) · [next: 03 Tasks](03-tasks-cores-timing.md) →
