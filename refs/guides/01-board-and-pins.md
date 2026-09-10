# 01 — Board and pins

**Goal:** treat the Waveshare netlist as law, then never trust a demo `sdkconfig` again.

**PDF:** [`../ESP32-S3-LCD-1.54-Schematic.pdf`](../ESP32-S3-LCD-1.54-Schematic.pdf)

**Wiki (orientation only):** https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54

## How to read the schematic

Walk the pages in this order. Tick each net against `architecture.md` §2. If they disagree, the schematic wins and architecture gets a one-line patch.

1. **ESP32-S3R8** — USB D−/D+ (19/20), flash, PSRAM (in-package, you will not see an external PSRAM chip), strapping.
2. **LCD connector** — `LCD_CS/CLK/DIN/DC/RST`, `LED_A/LED_K` (backlight). Confirm 4-wire SPI, **no MISO, no TE**.
3. **Touch** — `TP_SDA/SCL/INT/RST` on the same I2C as IMU/codec.
4. **IMU** — `IMU_SDA/SCL`, INT.
5. **Codec + amp** — ES8311 I2S (`MCLK/BCLK/LRCK/DSDIN`), ES7210 present, NS4150B `CTRL`.
6. **Battery** — ETA6098, `BAT_EN`, `BAT_ADC` divider, `CHG_STAT`, `KEY_PWR`, MX1.25 batt and **speaker**.
7. **TF** — present. Unused in the frame loop. Do not init in v1.

## Pin law (architecture §2)

Confirm these GPIO numbers on the schematic once. Then they are constants in firmware.

| Function | GPIO | Trap |
| :--- | ---: | :--- |
| LCD CS / CLK / MOSI | 21 / 38 / 39 | SPI3, write-only. No MISO. |
| LCD DC / RST / BL | 45 / 40 / 46 | **45 and 46 are strapping pins.** See guide 02. |
| I2C SCL / SDA | 41 / 42 | One bus: QMI8658, CST816, ES8311, ES7210 |
| Touch INT / RST | 48 / 47 | IRQ-driven. Do not poll in the IMU task. |
| IMU INT | 6 | FIFO watermark |
| PA enable | 7 | NS4150B CTRL. High only while a voice is live |
| I2S | 8 / 9 / 10 / 11 / 12 | MCLK / BCLK / WS / DIN / DOUT. DIN unused v1 |
| BAT_EN | 2 | **Hold high** or the board dies on battery |
| VBAT ADC / charging | 1 / 3 | Housekeeping. Not a PMIC. |
| PWR / PLUS / BOOT | 5 / 4 / 0 | PWR long-press = latch off. PLUS = recenter |
| USB D− / D+ | 19 / 20 | Native USB Serial/JTAG. No CH340. |

## Shared I2C (the real constraint)

Four slaves, one pair of wires, **one software owner on Core 0**.

| Device | Typical 7-bit addr | When you talk |
| :--- | :--- | :--- |
| QMI8658 | 0x6A or 0x6B (SA0) | 100 Hz FIFO drain |
| CST816 | 0x15 | On INT only |
| ES8311 | 0x18 | Play start / stop / volume — never inside the IMU drain |
| ES7210 | (leave uninited) | v1: do not probe, do not clock its I2S |

If codec init and IMU drain share the bus without a mutex (or a single owner task), the complementary filter stalls. Architecture forbids that.

## What the wiki is good for

- Photo of connectors (MX1.25 speaker is **TBD at bring-up** — open header is not a missing driver).
- SPI timing cartoon (mode 0, MSB first, DC = command/data). Confirm mode 0 vs 3 on the real panel (`architecture.md` checklist).

## What the wiki is bad for

Arduino, LVGL, XiaoZhi, 24 kHz duplex, TF MP3. Pin maps from [waveshareteam/ESP32-S3-Touch-LCD-1.54](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.54) are fair game; their `01_factory` audio graph is not.

## Bring-up test (step 1)

USB-CDC prints hello. GPIO2 (`BAT_EN`) driven high **before** anything else that can block. Unplug USB with a battery attached: the board must stay alive. If it dies, you dropped `BAT_EN`.
