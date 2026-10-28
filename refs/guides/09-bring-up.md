# 09 — Bring-up checklist

This is the lab sequence from `architecture.md` §15. The silicon procedures (pins, PDFs, BAT_EN) are in **[jpgma/esp32-s3 guide 09](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md)**. This page gives the **habitat** checkpoint for each of those steps.

Use the **pins** from the Waveshare examples. Do not keep LVGL, XiaoZhi or TF audio in the tree.

| Step | ESPET test | Hardware |
| ---: | :--- | :--- |
| 1 | `BAT_EN` high, USB-CDC hello, octal PSRAM at 80 MHz. The board stays on with battery power when you disconnect USB. | [esp32-s3 01/02/08](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md) |
| 2 | Backlight and a full-screen fill. SPI µs at 40 MHz and at 80 MHz. Decide mode 0 or mode 3. | [esp32-s3 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) |
| 3 | Indexed FB and a dirty-rect dummy sprite. GRAM keeps its contents when you stop DMA. | [guide 03 stub](./03-display-st7789.md) · architecture §3 |
| 4 | Complementary filter → gravity and `jerk`. **Live Nest slab.** Tilt does **not** turn the camera. A shake makes a debug core jump. **Product test.** | [04 IMU](./04-imu-qmi8658.md) [05 fusion](./05-fusion-gravity-camera.md) |
| 5 | One weighted mesh, six bones, two influences, flat N·L. A debug clip turns a bone. The room does not move. | [05](./05-fusion-gravity-camera.md) |
| 6 | Core spring and FK. Joint poke (S). A small number of rigid toys. Door cut Nest ↔ Hall, with `full_frame` one time. | [06 touch stub](./06-touch-cst816.md) |
| 7 | Pose mailbox. A sim that stops for short periods does not move the deadline. An empty mask when the pose is the same. Overlay: count of awake rigids. | [08 power stub](./08-power-battery.md) |
| 8 | ES8311 ACK, a PA pulse, a 12 kHz sine, PA low. Then a collision impact and one chirp in the play buffer. TX-done, then the PA goes low. | [07 audio stub](./07-audio-es8311.md) |
| 9 | UDP cortex last (inbox only). The lizard policy comes later. | architecture §13 |

## Rules that always apply (if one is broken, the step fails)

- Core 1 never waits for Core 0, I2C, I2S or Wi-Fi.
- **One I2C owner task.** The GPIO6 and GPIO48 ISRs only post. No ES8311 writes during the IMU drain. The sim posts start, stop and volume.
- World down is gravity. The pet does not lean with the glass.
- The room sets the camera. Tilt does not turn the camera. `full_frame` is a clip flag. The pet is one skinned mesh.
- The PA and the I2S clocks are off when DMA is idle. PA wake ≥35 ms, cold ≥120 ms.
- **Never probe** the ES7210. Do not initialize the TF card. GPIO18 is not a spare pin.
- Bluetooth is off. Wi-Fi is off, except for the cortex.
- Set `BAT_EN` high before all blocking work.

Hello in `board-sim`: a click is a CST816 tap (debug XY). A drag tilts the IMU.

## After the hardware is available

The behavior (wander, wave, sleep, think) is **TBD**. Do not invent a personality in the engine. The spatial hooks (Nest, Play, Hall, Yard) and these items are sufficient: `clip_id`, `emotion`, `vox_id`, `jerk`, `imu_evt`, one impact and one chirp.
