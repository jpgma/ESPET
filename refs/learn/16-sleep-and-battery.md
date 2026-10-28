# 16 — Sleep and battery

Silicon: **[jpgma/esp32-s3 guide 08](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/08-power-battery.md)**.

ESPET product: the 8 h target is parked. When the pose is the same, do not use SPI. Before light sleep or deep sleep, wait for TX-done. Then set the PA low. Refer to [`architecture.md` §14](../../architecture.md#14-power-8-h-parked).
