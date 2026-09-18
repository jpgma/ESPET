# 09 — Bring-up checklist

Lab order from `architecture.md` §15, with the PDF to have open and the test that means “this step is done.”

Steal **pins** from Waveshare examples. Do not leave LVGL, XiaoZhi, or TF audio in the tree.

| Step | Test | Guides | PDFs on the desk |
| ---: | :--- | :--- | :--- |
| 1 | `BAT_EN` high, USB-CDC hello, octal PSRAM 80 MHz in boot log, battery survives unplug | [01](./01-board-and-pins.md) [02](./02-soc-memory-smp.md) [08](./08-power-battery.md) | Schematic, S3 datasheet |
| 2 | Backlight PWM + full-screen fill. Print SPI µs at 40 vs 80 MHz. SPI mode 0 vs 3 settled | [03](./03-display-st7789.md) | ST7789V2, TRM SPI |
| 3 | Indexed FB + dirty-rect dummy sprite. GRAM holds when you stop DMA | [03](./03-display-st7789.md) | ST7789V2 |
| 4 | Complementary filter → gravity / `jerk`. **Authored Nest backdrop.** Tilt does **not** orbit. Shake hops a debug mass. **Product test.** | [04](./04-imu-qmi8658.md) [05](./05-fusion-gravity-camera.md) | QMI8658A, Madgwick ICORR |
| 5 | One L2 part, 4 idle yaw sheets, blit at `project(pos)` while `view_idx` follows **pet yaw**. Room stays put | [03](./03-display-st7789.md) [05](./05-fusion-gravity-camera.md) | — |
| 6 | Six springs + one idle rest. Sphere poke (S). Play toys. Dummy Hall L0 blob. Door cut | [06](./06-touch-cst816.md) | CST816T registers |
| 7 | Battery idle / GRAM-hold (springs/toys settled, `fx_live==0`). Studio vs battery backlight | [08](./08-power-battery.md) | ETA6098 |
| 8 | ES8311 ACK, PA pulse, 12 kHz sine, PA low. Then `SfxEvt` from collide; two voices; tail then PA drop | [07](./07-audio-es8311.md) | ES8311 user guide, NS4150B |
| 9 | UDP cortex last (inbox only). Lizard policy later | architecture §13 | IDF Wi-Fi (online) |

## Always-on invariants (fail the step if broken)

- Core 1 never waits on Core 0 / I2C / I2S / Wi-Fi.
- One I2C owner. No ES8311 writes inside the IMU drain.
- World down is gravity. Pet does not lean with the glass.
- Camera is room-authored; only yaw `view_idx` (0..3) is discrete.
- PA and I2S clocks off when both voices are dead.
- ES7210 and TF card not initialized.
- Bluetooth off. Wi-Fi off unless cortex.

## Architecture checklist (from §16) mapped here

Pins vs schematic; SPI mode; `BAT_EN`; PWR latch; VBAT; octal PSRAM; quad flash 80 MHz; cube world-up; authored camera (tilt does not orbit); shake hops + cooldown; indexed key color 0; dirty-rect 30 FPS; ES8311 + gated PA; `SfxEvt` threshold + cooldown; tail then sleep; radio-off toy is whole.

## After the hardware exists

Behaviour (wander, wave, sleep, think) is **TBD**. Do not invent a personality in the engine. Spatial hooks (Nest / Play / Hall / Yard) and (`clip_id`, `emotion`, `vox_id`, `jerk`, `imu_evt`, `SfxEvt`) are enough.
