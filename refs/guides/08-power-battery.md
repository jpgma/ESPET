# 08 — Power, battery, sleep

Board nets (`BAT_EN`, ETA6098, no PMIC): [jpgma/esp32-s3 guide 08](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/08-power-battery.md).

ESPET: the 8 h target is parked. An empty row mask lets a still pose send nothing. The present limit is the glass, not a 33 ms tick. Wait for TX-done before sleep. Refer to [`architecture.md` §14](../../architecture.md#14-power-8-h-parked).
