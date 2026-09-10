# 13 — Power and boot

← [12 springs](12-clips-springs-hitboxes.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 14 Touch](14-touch.md) →

**Read:** [guide 01](../guides/01-board-and-pins.md) · [guide 08](../guides/08-power-battery.md) · [guide 02](../guides/02-soc-memory-smp.md) (strapping, USB) · [ETA6098.pdf](../power/ETA6098.pdf) (no I2C; STAT only) · schematic battery page · [architecture 2 and 14](../../architecture.md#2-hardware-map)

**When:** the day the board lands, this becomes step 1 — *before* you care about the cube. Read it now so `app_main` is already in the right order.

There is no AXP-style PMIC. You program GPIOs and an ADC.

## First lines of `app_main`

1. Drive **GPIO2 `BAT_EN` high**. If you block before this with a battery attached and USB out, the board dies.
2. USB [CDC](../glossary.md#usb-cdc) log. No CH340 chip: D−/D+ are GPIO19/20. `idf.py -p COMx flash monitor`.
3. Confirm octal [PSRAM](../glossary.md#psram) 80 MHz in the boot log (`PSRAM: 8192K`). sdkconfig: `CONFIG_SPIRAM_MODE_OCT`, `CONFIG_SPIRAM_SPEED_80M`, quad flash 80 MHz **not** OPI.

[Strapping](../glossary.md#strapping-pin): GPIO45 = LCD DC, GPIO46 = backlight. After reset they are yours. Do not leave 46 floating.

## Battery housekeeping

| Net | GPIO | Job |
| :--- | ---: | :--- |
| `BAT_EN` | 2 | Hold high; [RTC](../glossary.md#rtc-sram) GPIO hold through deep sleep |
| `BAT_ADC` | 1 | Divider → [ADC](../glossary.md#adc) oneshot. Use the schematic ratio |
| `CHG_STAT` | 3 | ETA6098 STAT (charge / done). USB in ≈ studio |
| `KEY_PWR` | 5 | Long-press **latch off** (board circuit, not a charger command) |
| PLUS | 4 | Recenter, not volume |
| BOOT | 0 | Download strap |

[ETA6098](../glossary.md#eta6098): 4.2 V Li-ion switcher, ~1 µA on BAT when idle. You do not set charge current in software.

Studio vs battery: USB = bright, 30 FPS, no light-sleep, cortex allowed. Unplug = dim PWM, radio off. Always hold `BAT_EN`.

## `idf.py`

ESP-IDF ≥ 5.5, target `esp32s3`. Fake `board-sim` headers are **not** on that include path. Same `firmware/` sources plus `sdkconfig`.

Bluetooth **off**. Wi-Fi off until cortex. USB CDC on boot (`CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG`).

Serial/JTAG **dies in sleep**. Debug on USB means studio mode.

## Checkpoint (silicon)

Battery attached, unplug USB: still running. Log `WHO_AM_I` over CDC. VBAT prints a plausible voltage. PWR long-press latches off. PSRAM line present.

Sim: you cannot test this. Keep `BAT_EN` in the real `app_main` anyway so you never ship a build that forgets it.

## When the board arrives

This *is* the “when.” Architecture §15 step 1 + [guide 09](../guides/09-bring-up.md) row 1.

← [12 springs](12-clips-springs-hitboxes.md) · [next: 14 Touch](14-touch.md) →
