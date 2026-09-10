# 02 — SoC, memory, SMP

**Goal:** know enough ESP32-S3 to put framebuffers in DRAM, atlas in flash/PSRAM, USB CDC on boot, and two cores that do not share locks in the hot path.

## PDFs

| File | Read |
| :--- | :--- |
| [`../soc/esp32-s3_datasheet_en.pdf`](../soc/esp32-s3_datasheet_en.pdf) | Chip variants (**S3R8** = 8 MB in-package octal PSRAM), pin mux, **strapping**, ADC1 on GPIO1, USB 19/20 |
| [`../soc/esp32-s3_technical_reference_manual_en.pdf`](../soc/esp32-s3_technical_reference_manual_en.pdf) | **The book.** Do not read cover-to-cover. Chapters below. |
| [`../soc/esp32-s3_errata_en.pdf`](../soc/esp32-s3_errata_en.pdf) | Skim USB, cache, light-sleep. Note USB-OTG download ≠ Serial/JTAG |
| [`../soc/esp32-s3_hardware_design_guidelines_en.pdf`](../soc/esp32-s3_hardware_design_guidelines_en.pdf) | Strapping, flash/PSRAM, USB. You are not laying out a board; you are avoiding strapping foot-guns |
| [`../memory/W25Q128JV.pdf`](../memory/W25Q128JV.pdf) | Optional. Schematic marking. Day-to-day you talk to it through IDF SPI flash / XIP |

## TRM chapters that map to ESPET

| Topic | Why |
| :--- | :--- |
| GPIO + RTC GPIO | `BAT_EN` hold through deep sleep; INT edges; PA gate |
| SPI + GDMA | ST7789 write-only DMA. Color path must be DRAM (or bounce) |
| I2C | One master, several slaves |
| I2S | TX to ES8311, MCLK out on GPIO8 |
| USB Serial/JTAG | CDC + flash + OpenOCD on 19/20 |
| MSPI / cache / PSRAM | Octal 80 MHz. Wi-Fi DMA **cannot** live in PSRAM |
| SAR ADC | VBAT |
| System timer / CCOUNT | Core 1 33.3 ms lock (`architecture.md` §4) |
| Sleep / PMU | Light-sleep only if mixer idle |
| Interrupt matrix | GPIO6 IMU, GPIO48 touch |

Skip: Bluetooth, RMT, MCPWM, LCD_CAM RGB (this panel is SPI, not RGB), USB-OTG (this board uses Serial/JTAG).

## Strapping (GPIO45 / GPIO46)

Datasheet strapping table. On this board:

- **GPIO45 = LCD DC**
- **GPIO46 = backlight**

At reset both pins have a strapping meaning. Your first GPIO config must not fight download mode. After boot they are ordinary outputs. Do not leave 46 floating if you care about a black screen vs a white flash.

GPIO0 is BOOT. GPIO3 is also a strap on some S3 packages — here it is `CHG_STAT`; confirm in the datasheet vs schematic.

## sdkconfig (architecture §2)

```
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_SPEED_80M=y
# quad flash 80 MHz, NOT OPI
CONFIG_FREERTOS_HZ=1000
# Bluetooth off
# Wi-Fi started only in cortex mode
# USB CDC on boot  (CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
```

IDF:

- [External RAM](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-guides/external-ram.html)
- [Memory types](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-guides/memory-types.html)
- [USB Serial/JTAG console](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-guides/usb-serial-jtag-console.html)
- [FreeRTOS SMP](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-guides/freertos-smp.html)
- [Cache / DMA sync](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/system/mm_sync.html)

## Memory law (architecture §3)

| Region | Put here | Never here |
| :--- | :--- | :--- |
| Internal DRAM | Two indexed-8 FBs (2×57.6 KB), RGB565 scanline bounce, `Bodies`, seqlock, `SfxEvt` ring, two synth voices, 256-sample mix bounce, blit inner loop | Wi-Fi DMA buffers if you also keep dual RGB565 (you will not) |
| Octal PSRAM | Atlas pixels **if** XIP cache thrash shows up | Framebuffer, mixer, Wi-Fi DMA |
| 16 MB quad NOR | Firmware, clips, `sprite_id`, atlas, 20 vertices, `SynthPatch[]`. XIP for cold tables | PCM in v1 |
| RTC SRAM | Hunger, happy, sleep, last emotion | Anything in the 33 ms loop |

Wi-Fi DMA cannot live in PSRAM. That is why the product is indexed-8 in DRAM, not dual RGB565 + radio.

`DRAM_ATTR` on `g_shared[]` and `g_sfx[]`. `_Atomic` seqlock — **`volatile` is not a barrier on Xtensa SMP** (`architecture.md` §5).

## Two cores

| Core | Jobs |
| :--- | :--- |
| 0 | IMU 100 Hz, touch IRQ, lizard 20 Hz, mixer, backlight/VBAT, optional Wi-Fi |
| 1 | One pinned task: snapshot → springs → camera → dirty SPI → WFI until 33.3 ms |

Core 1 **never** waits on Core 0, I2S, or the radio. Sound is fire-and-forget `SfxEvt`.

CCOUNT / GPTimer lock the 30 Hz loop. IDF: [GPTimer](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/gptimer.html).

## USB

This board has **no USB-UART bridge**. GPIO19/20 are USB D−/D+. Flash with `idf.py -p COMx flash`. Serial/JTAG **does not work in deep sleep** — relevant when you add face-down sleep.

## Bring-up test (step 1 continued)

Print `esp_ptr_internal` vs `esp_ptr_external_ram` on a PSRAM alloc. Confirm octal 80 MHz in boot log (`PSRAM: 8192K`). USB CDC survives a reset without a CH340 driver.
