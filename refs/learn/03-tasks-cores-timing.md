# 03 — Tasks, cores, timing

← [02 buses](02-how-chips-talk.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 04 Vectors](04-vectors-matrices-camera.md) →

**Read:** [architecture 4. Core allocation](../../architecture.md#4-core-allocation) · [architecture 5. Inter-core state](../../architecture.md#5-inter-core-state-seqlock-dram-only) · [guide 02](../guides/02-soc-memory-smp.md) (SMP + memory law)

The ESP32-S3 has **two** [Xtensa](../glossary.md#xtensa) [cores](../glossary.md#core) at up to 240 MHz. [FreeRTOS](../glossary.md#freertos) is the tiny OS that runs your functions as [tasks](../glossary.md#task).

## `app_main` is already a task

When IDF boots, it calls `app_main`. In [firmware/main.c](../../firmware/main.c) the `for (;;)` loop *is* the program: read IMU, fill pixels, `vTaskDelay(33 ms)`.

`vTaskDelay` **blocks**: this task sleeps, other tasks can run. That is fine for a hello world. Architecture Core 1 still sleeps until `t0 + 33.3 ms`, but it must not block waiting for Core 0, Wi-Fi, or I2S.

[Tick](../glossary.md#tick): `CONFIG_FREERTOS_HZ=1000` means 1 ms ticks. `pdMS_TO_TICKS(33)` is ~33 ticks.

## Two cores, two jobs

| Core | What |
| :--- | :--- |
| **0** | [IMU](../glossary.md#imu) 100 Hz, touch [IRQ](../glossary.md#irq), lizard 20 Hz, mixer, backlight, optional Wi-Fi |
| **1** | One pinned task: snapshot → springs → camera → dirty SPI → wait for 33.3 ms |

**Golden rule 1:** Core 1 never waits on Core 0, Wi-Fi, or the LLM.

[Pinning](../glossary.md#pinning): `xTaskCreatePinnedToCore(..., 1)` so the body loop never migrates. Priorities: IMU 12, touch 11, mixer 7, housekeeping 5. Wi-Fi (when on) sits high inside IDF. Your tasks stay below that or they starve the radio — and you do not want the radio in the frame path anyway.

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
| Clip + springs + camera | &lt; 0.5 ms |
| Room quads | &lt; 1 ms |
| 5 blits | 0.5–2 ms |
| SPI DMA | ~7–12 ms dirty |
| Slack | [WFI](../glossary.md#wfi) |

Lock with [CCOUNT](../glossary.md#ccount) (CPU cycle counter) or a [GPTimer](../glossary.md#gptimer). If the frame is late, you still must not spin at 240 MHz drawing a static image. Idle → skip SPI ([GRAM](../glossary.md#gram) holds).

## Seqlock (how cores share)

Core 0 writes a [snapshot](../glossary.md#seqlock) `SharedSnap`. Core 1 copies one coherent slot per tick and never peeks again that frame.

```c
/* idea, not a copy-paste API */
uint32_t s1 = slot->seq;
/* read fields */
uint32_t s2 = slot->seq;
if (s1 != s2 || (s1 & 1)) { /* writer was in the middle; retry */ }
```

Odd `seq` = write in progress. Two slots so the writer can fill the other one.

**[`volatile`](../glossary.md#volatile) is not a barrier** on [SMP](../glossary.md#smp) Xtensa. Use `_Atomic` / `atomic_load` / `atomic_store` (architecture §5).

Sound the other way: Core 1 **never waits**. It pushes `SfxEvt` into an 8-deep ring; if full, drop oldest. Core 0 mixes.

## `DRAM_ATTR`

Shared state lives in internal [DRAM](../glossary.md#dram), not [PSRAM](../glossary.md#psram). Wi-Fi DMA cannot live in PSRAM. The blit inner loop never touches PSRAM. See [guide 02](../guides/02-soc-memory-smp.md).

## Checkpoint

You can draw Core 0 vs Core 1 on paper and say what is forbidden (Core 1 blocking, I2C in ISR, `volatile` as a lock).

## When the board arrives

Pin tasks for real. USB [CDC](../glossary.md#usb-cdc) logs which core a task is on (`xPortGetCoreID()`). Confirm IMU drain is Core 0.

← [02 buses](02-how-chips-talk.md) · [next: 04 Vectors](04-vectors-matrices-camera.md) →
