# 03 — Tasks, cores, timing

← [02 buses](./02-how-chips-talk.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 04 Vectors](./04-vectors-matrices-camera.md) →

**Read:** [architecture 4. Core allocation](../../architecture.md#4-core-allocation) · [architecture 5. Pose mailbox](../../architecture.md#5-pose-mailbox-dram-only) · [guide 02](../guides/02-soc-memory-smp.md) (SMP + memory law)

The ESP32-S3 has **two** [Xtensa](../glossary.md#xtensa) [cores](../glossary.md#core) at up to 240 MHz. [FreeRTOS](../glossary.md#freertos) is the tiny OS that runs your functions as [tasks](../glossary.md#task).

## `app_main` is already a task

When IDF boots, it calls `app_main`. In [firmware/main.c](../../firmware/main.c) the `for (;;)` loop *is* the program: read IMU, fill pixels, `vTaskDelay(33 ms)`.

`vTaskDelay` **blocks**: this task sleeps, other tasks can run. That is fine for a hello world. Architecture Core 1 waits for an absolute 33.3 ms deadline. It must not block waiting for Core 0, Wi-Fi, or I2S. A stalled sim holds the last pose.

[Tick](../glossary.md#tick): `CONFIG_FREERTOS_HZ=1000` means 1 ms ticks. `pdMS_TO_TICKS(33)` is ~33 ticks.

## Two cores, two jobs

| Core | What |
| :--- | :--- |
| **0** | [IMU](../glossary.md#imu) 100 Hz, touch [IRQ](../glossary.md#irq), lizard 20 Hz, **sim** (core spring, bones, rigids, particles), mixer, backlight, optional Wi-Fi |
| **1** | One pinned task: load pose → interpolate to the deadline → skin + raster → dirty SPI. No physics |

**Golden rule 1:** Core 1 never waits on Core 0, Wi-Fi, or the LLM.

[Pinning](../glossary.md#pinning): `xTaskCreatePinnedToCore(..., 1)` so present never migrates. Priorities: IMU 12, touch 11, mixer 7, **sim 6** (under the mixer), housekeeping 5. Wi-Fi (when on) sits high inside IDF. A long rigid-body solve must not starve I2S.

`board-sim` today: **one thread**. Write the code as if two cores exist. A seqlock still works with one reader and one writer on the same thread; you will feel the split when the sim grows.

## Interrupts

An [ISR](../glossary.md#isr) runs because a pin changed (IMU FIFO watermark, touch INT). Rules:

- Keep it tiny. Set a flag or give a semaphore.
- Do **not** talk [I2C](../glossary.md#i2c) inside the ISR.
- A task at prio 12 then drains the FIFO.

That is “deferred work.” Polling the IMU every 33 ms (current `main.c`) is a training wheel. Lesson 08 moves toward FIFO; hardware lesson 13 finishes the INT.

## The 33.3 ms budget

30 [FPS](../glossary.md#fps) = 1000/30 ≈ 33.3 ms per frame. Architecture times (40 MHz SPI):

| Slice | Time |
| :--- | :--- |
| Core spring + FK + rigids (Core 0) | 1–4 ms typical |
| Skin + raster (Core 1) | ~1–3 ms dirty; ~4–10 ms full frame |
| SPI DMA | dirty rows, or ~23 ms when `full_frame` |
| Slack | wait for the deadline |

Lock with [CCOUNT](../glossary.md#ccount) or a [GPTimer](../glossary.md#gptimer). The tick is an absolute deadline, not a delay after the work. If the pose matches the last one, the row mask is empty and you skip SPI ([GRAM](../glossary.md#gram) holds). That is the frame period, not an 8 h gate.

## Pose mailbox (how cores share)

Core 0 writes a [pose](../../architecture.md#5-pose-mailbox-dram-only): timestamp, camera, six bone matrices, rigid instances. Three slots. Core 1 loads the latest and the previous, and interpolates to **this** deadline. It never peeks again that frame. If the sim misses a publish, Core 1 draws the last complete pose. It does not extrapolate.

```c
/* idea, not a copy-paste API */
uint32_t s1 = slot->seq;
/* read fields */
uint32_t s2 = slot->seq;
if (s1 != s2 || (s1 & 1)) { /* writer was in the middle; retry */ }
```

Odd `seq` = write in progress. Two slots so the writer can fill the other one.

**[`volatile`](../glossary.md#volatile) is not a barrier** on [SMP](../glossary.md#smp) Xtensa. Use `_Atomic` / `atomic_load` / `atomic_store` (architecture §5).

Sound stays on Core 0. The sim pushes `SfxEvt` into an 8-deep ring and does not call the mixer. If the ring is full, drop oldest. The mixer is higher priority than the sim.

## `DRAM_ATTR`

The pose, the indexed frame, and the DMA bands live in internal [DRAM](../glossary.md#dram), not [PSRAM](../glossary.md#psram). Wi-Fi DMA cannot live in PSRAM. The raster inner loop never touches PSRAM. See [guide 02](../guides/02-soc-memory-smp.md).

## Checkpoint

You can draw Core 0 vs Core 1 on paper and say what is forbidden (Core 1 blocking, I2C in ISR, `volatile` as a lock).

## When the board arrives

Pin tasks for real. USB [CDC](../glossary.md#usb-cdc) logs which core a task is on (`xPortGetCoreID()`). Confirm IMU drain is Core 0.

← [02 buses](./02-how-chips-talk.md) · [next: 04 Vectors](./04-vectors-matrices-camera.md) →
