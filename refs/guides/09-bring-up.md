# 09 — Bring-up checklist

Lab order from `architecture.md` §15. Silicon how-to (pins, PDFs, BAT_EN) is **[jpgma/esp32-s3 guide 09](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md)**. This page maps those steps to **habitat** checkpoints.

Steal **pins** from Waveshare examples. Do not leave LVGL, XiaoZhi, or TF audio in the tree.

| Step | ESPET test | Hardware |
| ---: | :--- | :--- |
| 1 | `BAT_EN` high, USB-CDC hello, octal PSRAM 80 MHz, battery survives unplug | [esp32-s3 01/02/08](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md) |
| 2 | Backlight + full-screen fill. SPI µs at 40 vs 80. Mode 0 vs 3 settled | [esp32-s3 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) |
| 3 | Indexed FB + dirty-rect dummy sprite. GRAM holds when you stop DMA | [guide 03 stub](./03-display-st7789.md) · architecture §3 |
| 4 | Complementary filter → gravity / `jerk`. **Live Nest slab.** Tilt does **not** orbit. Shake hops a debug core. **Product test.** | [04 IMU](./04-imu-qmi8658.md) [05 fusion](./05-fusion-gravity-camera.md) |
| 5 | One weighted mesh, six bones, two influences, flat N·L. A debug clip turns a bone. The room stays put | [05](./05-fusion-gravity-camera.md) |
| 6 | Core spring + FK. Joint poke (S). A few rigid toys. Door cut Nest ↔ Hall, `full_frame` once | [06 touch stub](./06-touch-cst816.md) |
| 7 | Pose mailbox. A hitching sim does not move the deadline. Empty mask when the pose matches. Overlay: awake rigid count | [08 power stub](./08-power-battery.md) |
| 8 | ES8311 ACK, PA pulse, 12 kHz sine, PA low. Then `SfxEvt` from collide; two voices; tail then PA drop | [07 audio stub](./07-audio-es8311.md) |
| 9 | UDP cortex last (inbox only). Lizard policy later | architecture §13 |

## Always-on invariants (fail the step if broken)

- Core 1 never waits on Core 0 / I2C / I2S / Wi-Fi.
- One I2C owner. No ES8311 writes inside the IMU drain.
- World down is gravity. Pet does not lean with the glass.
- Camera is room-authored; tilt does not orbit; `full_frame` is a clip flag. The pet is one skinned mesh.
- PA and I2S clocks off when both voices are dead.
- ES7210 and TF card not initialized.
- Bluetooth off. Wi-Fi off unless cortex.

## After the hardware exists

Behaviour (wander, wave, sleep, think) is **TBD**. Do not invent a personality in the engine. Spatial hooks (Nest / Play / Hall / Yard) and (`clip_id`, `emotion`, `vox_id`, `jerk`, `imu_evt`, `SfxEvt`) are enough.
