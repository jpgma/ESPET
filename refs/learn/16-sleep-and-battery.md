# 16 — Sleep and battery

Silicon: **[jpgma/esp32-s3 guide 08](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/08-power-battery.md)**.

ESPET product: 8 h is parked. Skip SPI when the pose matches. Wait for TX-done, then PA low, before light-sleep or deep sleep. [`architecture.md` §14](../../architecture.md#14-power-8-h-parked).
