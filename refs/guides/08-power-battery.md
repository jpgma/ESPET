# 08 — Power, battery, sleep

**Goal:** stay alive on battery (`BAT_EN`), know approximate VBAT, PWR long-press latches off, light-sleep only when the mixer is idle. Strive ~8 h dim, radio off — not a hard cap that kills chirps.

There is **no AXP PMIC** on this board. You program GPIOs and ADC.

## PDFs

| File | Read |
| :--- | :--- |
| [`../ESP32-S3-LCD-1.54-Schematic.pdf`](../ESP32-S3-LCD-1.54-Schematic.pdf) | Battery page: ETA6098, divider, `BAT_EN`, `KEY_PWR`, `CHG_STAT` |
| [`../power/ETA6098.pdf`](../power/ETA6098.pdf) | 4.2 V Li-ion switcher. **No I2C registers.** STAT, charge current from RISET, 1 µA BAT quiescent |
| [`../soc/esp32-s3_technical_reference_manual_en.pdf`](../soc/esp32-s3_technical_reference_manual_en.pdf) | RTC GPIO hold, sleep, SAR ADC |
| [`../soc/esp32-s3_datasheet_en.pdf`](../soc/esp32-s3_datasheet_en.pdf) | ADC1 channel on GPIO1 |

IDF:

- [ADC oneshot](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/adc_oneshot.html)
- [ADC calibration](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/adc_calibration.html)
- [Sleep modes](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/system/sleep_modes.html)
- [Power management](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/system/power_management.html)
- [GPIO hold](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/gpio.html)

## Nets

| Net | GPIO | Job |
| :--- | ---: | :--- |
| `BAT_EN` | 2 | **Drive high in `app_main` before anything else.** Hold through deep sleep (`rtc_gpio_hold`). Drop = board dies on battery |
| `BAT_ADC` | 1 | Divider. Convert with the schematic ratio (do not assume ×2). Housekeeping 1–10 Hz |
| `CHG_STAT` | 3 | ETA6098 STAT. Charging vs done vs fault — read the charger sheet + schematic resistor. USB in ≈ studio mode |
| `KEY_PWR` | 5 | Long-press = latch off (board circuit + GPIO). Not a charger command |
| PLUS | 4 | Recenter. Not volume |
| BOOT | 0 | Download / strap |

ETA6098: precharge / CC / CV, terminate, OVP. You do not set current in software. If STAT is open-drain, use a pull-up and debounce.

## Power personality (architecture §14)

USB plugged in = **studio**: bright backlight, 30 FPS, no light-sleep, cortex allowed, audio may be louder.

Unplug = battery: dim PWM, radio off, GRAM-hold, WFI, light-sleep **only if mixer idle**.

1000 mAh / 8 h ≈ 125 mA average. Strive. Chirps on battery are in. Idle I2S is not.

Levers: backlight 30–40%, dirty ST7789, CPU 240 MHz interacting then 80/160, Wi-Fi off, PA+I2S gated, ES7210 off, SD off.

## Sleep vs GRAM-hold vs audio tail

Three different things:

| State | Display | Audio | CPU |
| :--- | :--- | :--- | :--- |
| Interacting | 30 Hz dirty | PA as needed | 240 MHz |
| Idle, mixer live | GRAM holds (skip SPI) | Tail plays, PA high | Core 1 WFI; **no** chip light-sleep |
| Idle, mixer silent | GRAM holds | PA low, I2S clocks off, codec standby | Light-sleep allowed |
| Face-down / PWR / long idle | Off or GRAM | Finish tail, PA low, then deep sleep + RTC restore | `BAT_EN` held |

USB Serial/JTAG dies in sleep. Debug on USB is studio mode.

## RTC SRAM

Hunger, happy, sleep, last emotion (later `part_ids[6]`). Decay rates TBD with the lizard brain. The storage exists now.

## Bring-up test (step 1 + 7)

Battery attached, USB unplug: still running (`BAT_EN`). Print VBAT. PWR long-press: latch off. Face-down path: wait for mixer idle, PA low, then sleep; wake restores RTC fields. Measure mA later; do not tune 8 h until GRAM-hold and PA gate are real.
