# 15 — Audio ES8311 + play buffer

Silicon: **[jpgma/esp32-s3 guide 07](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/07-audio-es8311.md)**.

ESPET product:

- The sim on Core 0 renders one-shots into one **12 kHz** mono `int16` buffer (300 ms, 7200 bytes, internal DRAM). I2S DMA plays it.
- Impacts are procedural. The velocity changes the decay.
- Creature lines are **s8** chirps in flash. The sim expands them at the event.
- A second class adds its samples in front of the DMA read pointer.
- The last event of a class overwrites the previous one, with a 5 ms crossfade.
- A maximum of one event of each class occurs on each tick.
- The ES7210 is **off**.
- The PA is high only while DMA runs. TX-done stops the clocks. Then the PA goes low.

Refer to [`architecture.md` §9](../../architecture.md#9-audio). A 440 Hz sine was audible on this unit. Thus, the MX1.25 speaker and the NS4150B path operate. The simulator has no codec. Until silicon is available, a log of the `SfxEvt` is sufficient.
