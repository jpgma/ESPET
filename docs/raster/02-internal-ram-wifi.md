# 02 — Internal RAM with Wi-Fi

← [budget](./01-budget.md) · [index](./README.md) · [next: present](./03-cadenced-present.md) →

**Goal:** the indexed frame and both bounce bands live in internal DRAM while Wi-Fi is up, and you know how many internal bytes are left.

**Depends on:** task 1.

**Read:** [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md) · [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md) memory law · [bring-up](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md) step 1

## Why

A full RGB565 frame is 115200 bytes. Two of them plus the radio do not fit the 512 KB story. Wi-Fi DMA cannot live in PSRAM. The filler’s bytes and the SPI DMA bytes are internal. PSRAM is for meshes, later.

Bluetooth stays off so this frame has a chance to fit.

## What to produce

On the host, now:

- Constants for the indexed frame, the palette, and the two bands, taken from your `budget.md`.
- Storage that you can name as internal DRAM on the chip. Prefer one `static` buffer with a comment that says internal DRAM, or `heap_caps_malloc` with `MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA` for the bands. The indexed frame is CPU-only; the bands are what DMA reads.
- A boot log line that prints `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` and the address class of each buffer (`esp_ptr_internal` versus external).
- An sdkconfig sketch next to the log line: octal PSRAM 80 MHz, quad flash 80 MHz, Bluetooth off, Wi-Fi task pinned to core 0 (menuconfig “WiFi Task Core ID”).

The fake headers in `board-sim/fake_idf` do not provide heap caps or Wi-Fi. Guard the silicon-only calls so `sim.bat` still builds. A host build that prints the three sizes is enough here.

On the board, when you have it:

1. `BAT_EN` high before anything that can block ([power and boot](../learn/power-and-boot.md)).
2. Bring up Wi-Fi far enough that the driver has allocated (associated, or at least started). Leave Bluetooth compiled out.
3. Allocate or touch the three buffers after that.
4. Copy the free-internal number into the blank line in `budget.md`.

If the allocation fails, lower Wi-Fi RX/TX buffer counts and record what you changed. Do not move the indexed frame or the bands to PSRAM to make it pass.

## Done when

- Host: the sizes match `budget.md`, and `sim.bat` still builds.
- Silicon: the log shows all three buffers internal, Wi-Fi is up, Bluetooth is off, and `budget.md` has the remaining internal heap.
- That remaining number is still positive with the LCD driver and a small stack reserved. Write the number you saw, not a guess.

## Leave for later

No triangles, no present loop, no second indexed frame. A z-buffer is not reserved.
