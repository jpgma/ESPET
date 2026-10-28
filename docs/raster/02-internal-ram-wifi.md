# 02 — Internal RAM with Wi-Fi

← [budget](./01-budget.md) · [index](./README.md) · [next: present](./03-cadenced-present.md) →

**Objective:** put the indexed frame and the two bounce bands in internal DRAM while Wi-Fi is on. Find how many internal bytes remain.

**Prerequisites:** task 1.

**Read:** [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md) · [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md) memory rules · [bring-up](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md) step 1

## Reason

A full RGB565 frame is 115200 bytes. Two of these frames and the radio do not fit in 512 KB. Wi-Fi DMA cannot be in PSRAM. The filler bytes and the SPI DMA bytes are internal. PSRAM is for meshes, later.

Bluetooth stays off, so that this frame can fit.

## Output

On the host, now:

- Make constants for the indexed frame, the palette and the two bands. Use the values from your `budget.md`.
- Make storage that you can identify as internal DRAM on the chip. Use one `static` buffer with a comment that says "internal DRAM". Or, for the bands, use `heap_caps_malloc` with `MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA`. Only the CPU uses the indexed frame. DMA reads the bands.
- Add a boot log line. It prints `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` and the address class of each buffer (`esp_ptr_internal` or external).
- Write an sdkconfig sketch near the log line: octal PSRAM at 80 MHz, quad flash at 80 MHz, Bluetooth off, Wi-Fi task pinned to Core 0 (menuconfig "WiFi Task Core ID").

The fake headers in `board-sim/fake_idf` do not supply heap caps or Wi-Fi. Put a guard around the silicon-only calls, so that `sim.bat` continues to build. For this task, a host build that prints the three sizes is sufficient.

On the board, when it is available:

1. Set `BAT_EN` high before all operations that can block ([power and boot](../../refs/learn/13-power-and-boot.md)).
2. Start Wi-Fi until the driver has allocated its memory (associated, or at minimum started). Do not compile Bluetooth.
3. After that, allocate or write the three buffers.
4. Copy the free internal value into the empty line in `budget.md`.

If the allocation fails, decrease the number of Wi-Fi RX/TX buffers. Record the changes. Do not move the indexed frame or the bands to PSRAM to make the test pass.

## Complete when

- Host: the sizes agree with `budget.md`, and `sim.bat` continues to build.
- Silicon: the log shows that the three buffers are internal, Wi-Fi is on and Bluetooth is off. `budget.md` has the remaining internal heap.
- The remaining value is still more than zero with the LCD driver and a small stack reserved. Write the value that you saw, not an estimate.

## For later

No triangles, no present loop, no second indexed frame. Do not reserve a z-buffer.
