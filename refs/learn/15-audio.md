# 15 — Audio ES8311 + mixer

Silicon: **[jpgma/esp32-s3 guide 07](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/07-audio-es8311.md)**.

ESPET product: Core 0, **two voices**, 12 kHz, block 256, ES7210 **off**, PA gated, `SfxEvt` from collide. [`architecture.md` §9](../../architecture.md#9-audio). Sim has no codec — mixer into DRAM bounce is still valid.
