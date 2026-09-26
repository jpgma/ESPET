# 14 — Touch CST816

Silicon: **[jpgma/esp32-s3 guide 06](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/06-touch-cst816.md)**.

ESPET product: poke UV unprojects in S rooms; L uses screen-space slop. Double-tap is a lizard one-shot, **not** camera recenter. [`architecture.md` §8](../../architecture.md#8-physics-and-hitboxes).

`board-sim` fakes CST816 at I2C **0x15**: **click** (no drag) writes GestureID + XY and pulses GPIO48; **drag** is still the IMU. Hello reads the burst on INT and stashes poke UV — no sphere unproject yet. Enable `EnDClick` and `DisAutoSleep`. This glass is **CST816D**: `ChipID` `0xB6` at `0xA7`, project `0x27`, firmware `0x01`.
