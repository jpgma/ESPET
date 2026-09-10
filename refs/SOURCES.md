# Source inventory

Local copies of the chip manuals used by `architecture.md`. HTML ESP-IDF pages are **not** mirrored (they version with the SDK); the guides link v5.5.4.

## Downloaded

| File | Source |
| :--- | :--- |
| [ESP32-S3-LCD-1.54-Schematic.pdf](./ESP32-S3-LCD-1.54-Schematic.pdf) | [Waveshare](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.54/ESP32-S3-LCD-1.54-Schematic.pdf) |
| [soc/esp32-s3_datasheet_en.pdf](./soc/esp32-s3_datasheet_en.pdf) | [Espressif](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf) |
| [soc/esp32-s3_technical_reference_manual_en.pdf](./soc/esp32-s3_technical_reference_manual_en.pdf) | [Espressif](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf) |
| [soc/esp32-s3_errata_en.pdf](./soc/esp32-s3_errata_en.pdf) | [esp-chip-errata](https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32s3/esp-chip-errata-en-master-esp32s3.pdf) |
| [soc/esp32-s3_hardware_design_guidelines_en.pdf](./soc/esp32-s3_hardware_design_guidelines_en.pdf) | [esp-hardware-design-guidelines](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/esp-hardware-design-guidelines-en-master-esp32s3.pdf) |
| [display/ST7789V2.pdf](./display/ST7789V2.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/ST7789V2.pdf) |
| [imu/QMI8658A_Datasheet_Rev_A.pdf](./imu/QMI8658A_Datasheet_Rev_A.pdf) | [Waveshare / QST](https://files.waveshare.com/wiki/common/QMI8658A_Datasheet_Rev_A.pdf) |
| [imu/QMI8658C_datasheet_rev_0.9.pdf](./imu/QMI8658C_datasheet_rev_0.9.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/QMI8658C_datasheet_rev_0.9.pdf) |
| [touch/CST816T_register_v1.3.pdf](./touch/CST816T_register_v1.3.pdf) | [Hynitron / OSPTEK](https://admin.osptek.com/uploads/CST_816_T_v1_3_1_69c246954d.pdf) |
| [touch/CST816S_register_declaration.pdf](./touch/CST816S_register_declaration.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/CST816S_register_declaration.pdf) |
| [touch/CST816S_Datasheet_EN.pdf](./touch/CST816S_Datasheet_EN.pdf) | [Waveshare](https://files.waveshare.com/upload/5/51/CST816S_Datasheet_EN.pdf) |
| [touch/CST816D_datasheet_En_V1.3.pdf](./touch/CST816D_datasheet_En_V1.3.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/CST816D_datasheet_En_V1.3.pdf) |
| [audio/ES8311.DS.pdf](./audio/ES8311.DS.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/ES8311.DS.pdf) |
| [audio/ES8311.user.Guide.pdf](./audio/ES8311.user.Guide.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/ES8311.user.Guide.pdf) |
| [audio/ES7210-datasheet.pdf](./audio/ES7210-datasheet.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/ES7210-datasheet.pdf) |
| [audio/NS4150B.pdf](./audio/NS4150B.pdf) | [Nsiway via XON](https://xonstorage.z8.web.core.windows.net/pdf/nsiway_ns4150b_apr22_xonlink.pdf) |
| [power/ETA6098.pdf](./power/ETA6098.pdf) | [Waveshare](https://files.waveshare.com/wiki/common/ETA6098.pdf) |
| [memory/W25Q128JV.pdf](./memory/W25Q128JV.pdf) | Winbond W25Q128JVSIQ (schematic marking) |
| [fusion/madgwick_icorr2011.pdf](./fusion/madgwick_icorr2011.pdf) | Madgwick, ICORR 2011 (6-axis `updateIMU`) |

## Not mirrored (paywall / 403)

| Document | Why | What to use instead |
| :--- | :--- | :--- |
| Madgwick 2010 internal report | x-io.co.uk returned 403 | [madgwick_icorr2011.pdf](./fusion/madgwick_icorr2011.pdf) is the archival paper |
| Mahony, Hamel, Pflimlin, *Nonlinear Complementary Filters on SO(3)*, IEEE TAC 2008 | IEEE paywall | [guide 05](./guides/05-fusion-gravity-camera.md) derives the 6-axis filter ESPET actually needs. Paper: [DOI 10.1109/TAC.2008.923738](https://doi.org/10.1109/TAC.2008.923738) / [HAL](https://hal.science/hal-00488376) if you have access |

## Online (do not vendor-copy)

- Board wiki: https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54
- Demo repo (pins / init sequences only): https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.54
- ESP-IDF 5.5.4 for ESP32-S3: https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/index.html
