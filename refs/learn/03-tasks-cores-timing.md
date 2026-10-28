# 03 — Tasks, cores, timing

← [02 buses](./02-how-chips-talk.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 04 Vectors](./04-vectors-matrices-camera.md) →

**Read:** [architecture 4. Core allocation](../../architecture.md#4-core-allocation) · [architecture 5. Pose mailbox](../../architecture.md#5-pose-mailbox-dram-only) · [guide 02](../guides/02-soc-memory-smp.md) (SMP and memory rules)

The ESP32-S3 has **two** [Xtensa](../glossary.md#xtensa) [cores](../glossary.md#core) at a maximum of 240 MHz. [FreeRTOS](../glossary.md#freertos) is the very small OS that runs your functions as [tasks](../glossary.md#task).

## `app_main` is already a task

When IDF boots, it calls `app_main`. In [firmware/main/main.c](../../firmware/main/main.c), the `for (;;)` loop *is* the program: an indexed frame at the glass limit (`platform_pace(12500)`). It does not read the IMU.

`vTaskDelay` **blocks**: this task sleeps, and other tasks can run. For a hello program, this is satisfactory. In the architecture, Core 1 waits only for the glass limit, **12500 µs**. It must not block while it waits for Core 0, Wi-Fi or I2S. If the sim stops, Core 1 holds the last pose.

[Tick](../glossary.md#tick): `CONFIG_FREERTOS_HZ=1000` gives ticks of 1 ms. `pdMS_TO_TICKS(12)` is 12 ticks, near that limit.

## Two cores, two jobs

| Core | Jobs |
| :--- | :--- |
| **0** | [IMU](../glossary.md#imu) at 100 Hz, touch [IRQ](../glossary.md#irq), lizard brain at 20 Hz, **sim** (core spring, bones, rigids, particles, one-shots), backlight, optional Wi-Fi |
| **1** | One pinned task: load the pose → interpolate to the deadline → skin and raster → SPI of the dirty rows. No physics. |

**Golden rule 1:** Core 1 never waits for Core 0, Wi-Fi or the LLM.

[Pinning](../glossary.md#pinning): use `xTaskCreatePinnedToCore(..., 1)`, so that the present never moves to the other core. Priorities: I2C owner 13, IMU filter 12, **sim 6**, housekeeping 5. When Wi-Fi is on, it has a high priority in IDF. Playback uses DMA. Thus, a long rigid-body solve does not stop I2S.

Now, `board-sim` has **one thread**. Write the code as if two cores exist. A seqlock also operates with one reader and one writer on the same thread. You will see the division when the simulator becomes larger.

## Interrupts

An [ISR](../glossary.md#isr) runs because a pin changed (IMU FIFO watermark, touch INT). Rules:

- Keep the ISR very small. Set a flag or give a semaphore.
- Do **not** use [I2C](../glossary.md#i2c) in the ISR.
- After the ISR, a task at priority 12 drains the FIFO.

This is "deferred work". To poll the IMU in the present loop is only a temporary aid. The raster stub does not read the IMU. Lesson 08 moves toward the FIFO. Hardware lesson 13 completes the INT.

## The glass limit

There is no locked frame rate. 80 Hz is **12500 µs** between the starts of two frames. A slower frame runs at its own work time. A quiet frame does not use SPI. The sim step on Core 0 stays at 30 Hz. The architecture times at 80 MHz SPI:

| Slice | Time |
| :--- | :--- |
| Core spring, FK, rigids (Core 0) | Usually 1–4 ms |
| Skin and raster (Core 1) | Approximately 1–3 ms for dirty rows; approximately 4–10 ms for a full frame |
| SPI DMA | Dirty rows, or approximately 12.3 ms with `full_frame` |
| Slack | Only if the frame is complete in 12500 µs |

Lock the timing with [CCOUNT](../glossary.md#ccount) or a [GPTimer](../glossary.md#gptimer). The tick is an absolute deadline, not a delay after the work. If the pose is the same as the last pose, the row mask is empty and you do not use SPI ([GRAM](../glossary.md#gram) keeps its contents). This is the frame period, not an 8 h gate.

## Pose mailbox (how the cores share data)

Core 0 writes a [pose](../../architecture.md#5-pose-mailbox-dram-only): timestamp, camera, six bone matrices, rigid instances. There are three slots. Core 1 loads the latest pose and the previous pose. It interpolates to the start time of this frame. It does not read the mailbox again during that frame. If the sim misses a publish, Core 1 draws the last complete pose. It does not extrapolate.

```c
/* idea, not a copy-paste API */
uint32_t s1 = slot->seq;
/* read fields */
uint32_t s2 = slot->seq;
if (s1 != s2 || (s1 & 1)) { /* writer was in the middle; retry */ }
```

An odd `seq` means that a write is in progress. There are two slots, so that the writer can fill the other slot.

**[`volatile`](../glossary.md#volatile) is not a barrier** on [SMP](../glossary.md#smp) Xtensa. Use `_Atomic`, `atomic_load` and `atomic_store` (architecture §5).

Sound stays on Core 0. On that tick, the sim renders a maximum of one impact and one chirp into the [play buffer](../glossary.md#play-buffer), and then returns. I2S DMA plays the buffer. There is no mixer task.

## `DRAM_ATTR`

The pose, the indexed frame, the DMA bands and the play buffer are in internal [DRAM](../glossary.md#dram), not in [PSRAM](../glossary.md#psram). Wi-Fi DMA cannot be in PSRAM. The raster inner loop never reads PSRAM. Refer to [guide 02](../guides/02-soc-memory-smp.md).

## Checkpoint

You can draw Core 0 and Core 1 on paper. You can say what is prohibited: Core 1 that blocks, I2C in an ISR, and `volatile` as a lock.

## When the board arrives

Pin the tasks on the real cores. Use USB [CDC](../glossary.md#usb-cdc) to record the core of each task (`xPortGetCoreID()`). Make sure that the IMU drain is on Core 0.

← [02 buses](./02-how-chips-talk.md) · [next: 04 Vectors](./04-vectors-matrices-camera.md) →
