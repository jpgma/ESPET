# 16 — Sleep and battery

Silicon: **[jpgma/esp32-s3 guide 08](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/08-power-battery.md)**.

ESPET product: strive ~8 h dim; GRAM-hold when springs settled, RBs sleeping, `fx_live==0`; light-sleep only if mixer idle; finish audio tail before PA low. [`architecture.md` §14](../../architecture.md#14-power-8-h-strive).
