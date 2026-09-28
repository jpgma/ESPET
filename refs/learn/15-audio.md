# 15 — Audio ES8311 + play buffer

Silicon: **[jpgma/esp32-s3 guide 07](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/07-audio-es8311.md)**.

ESPET product: the sim on Core 0 renders one-shots into one **12 kHz** mono `int16` buffer (300 ms, 7200 bytes, internal DRAM). I2S DMA plays it. Impacts are procedural (velocity scales the decay). Creature lines are **s8** chirps in flash, expanded at the event. A second class adds ahead of the DMA read pointer. Last event of a class overwrites with a 5 ms crossfade. At most one of each class per tick. ES7210 **off**. PA high only while DMA is running. TX-done stops the clocks, then the PA drops. [`architecture.md` §9](../../architecture.md#9-audio). A 440 Hz sine was heard on this unit, so the MX1.25 speaker and NS4150B path work. Sim has no codec — logging the `SfxEvt` is enough until silicon.
