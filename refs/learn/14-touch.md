# 14 — Touch CST816

Silicon: **[jpgma/esp32-s3 guide 06](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/06-touch-cst816.md)**.

ESPET product: in S rooms, the poke UV is unprojected. L rooms use a screen-space tolerance. A double-tap is a lizard one-shot. It does **not** recenter the camera. Refer to [`architecture.md` §8](../../architecture.md#8-physics-and-hitboxes).

`board-sim` simulates the CST816 at I2C **0x15**:

- A **click** (no drag) writes GestureID and XY, and sends a pulse on GPIO48.
- A **drag** is still the IMU.

Hello reads the burst on INT and keeps the poke UV. It does not do the sphere unproject yet. Enable `EnDClick` and `DisAutoSleep`. This glass has a **CST816D**: `ChipID` `0xB6` at `0xA7`, project `0x27`, firmware `0x01`.
