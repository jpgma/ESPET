# 05 — Pose mailbox

← [one triangle](./04-one-triangle.md) · [index](./README.md) · [next: mesh and dirty rows](./06-mesh-clip-dirty.md) →

**Objective:** the simulations publish a pose and return. The present runs at the maximum rate that the frame permits, with the glass as the limit. If the publisher stops, the present holds the last pose.

**Prerequisites:** the clock from task 3. The triangle can stay on the screen as the moving item.

**Read:** [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md) (two cores and the `volatile` line) · [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md)

## Contract

A pose is a POD in `firmware/main/`. It has a timestamp in microseconds, a camera and a short list of instance transforms (a translation and a quaternion are sufficient). It has no pointers into sim memory. It has no ESP-IDF types.

There are three slots. Their size is in `budget.md`. The publisher does these steps:

1. It writes a free slot.
2. It stores the slot index with an atomic release (or a seqlock). `volatile` is not the barrier.

The renderer does these steps one time for each frame, on Core 1:

1. It loads the latest published slot with an atomic acquire.
2. It also keeps the previous pose.
3. It interpolates to the **frame start**, not to the time of the last sim step. Normalize the quaternion (nlerp is sufficient). Lerp the translation. Clamp the blend to the newest pose.
4. If the newest timestamp is older than the glass limit (**12500 µs**), it draws that pose without a change. Do not extrapolate.

The publisher never waits for the present. The renderer never waits for the publisher, for I2C or for Wi-Fi.

## Core allocation

| Work | Core |
| :--- | :--- |
| Present and interpolation | 1, pinned, no other task |
| Dummy publisher, and later the real simulations | 0 |
| Wi-Fi task | 0 (task 2) |
| I2C | 0, one owner. This dummy publisher does not use GPIO 41/42. |

The fake FreeRTOS of the simulator is a `Sleep`. It is not SMP. Prove the mailbox with a host test. Use a writer thread and a reader thread. Or, use a single-thread script that publishes three poses and makes sure that the reader never sees a torn transform. (Write a sentinel word first and last. The two words must be the same.)

## Hitch test

The writer sleeps in bursts of different lengths (tens of milliseconds, sometimes longer than the glass limit). The reader records the timestamps of each present start.

- While the frame takes less than 12500 µs, the gap between starts stays on the limit of task 3. The sleeps of the publisher do not add to the gap. The Windows thread clock is not accurate. Record the error. On the chip, the gap stays within **2 ms** of 12500 µs while the writer sleeps in bursts.
- When the work of the frame takes longer than the limit, the gap is that work time.
- During a stop that is longer than the glass limit, the interpolated pose does not change.
- When the writer starts again, the picture moves smoothly from the last held pose to the new pose. It does not jump to a partial write.

Write the gaps that you see in `budget.md`, as a note below the glass-limit line.

## Complete when

- The host test rejects a torn pose.
- If the publisher stops, the present holds the last complete pose.
- The present starts stay on the glass limit while the publisher stops for short periods, if the frame takes less time than that limit.
- The raster core still has no ESP-IDF include. Atomics from `stdatomic.h` are permitted.

## For later

No real IMU camera, no mesh file format. Task 6 uses this pose.
