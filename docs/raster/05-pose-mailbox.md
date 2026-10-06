# 05 — Pose mailbox

← [one triangle](./04-one-triangle.md) · [index](./README.md) · [next: mesh and dirty rows](./06-mesh-clip-dirty.md) →

**Goal:** simulations publish a pose and return. Present stays at the maximum the frame allows, capped at the glass, and a stalled publisher holds the last pose.

**Depends on:** task 3’s clock. The triangle can stay on screen as the thing that moves.

**Read:** [docs/soc.md](https://github.com/jpgma/esp32-s3/blob/main/docs/soc.md) two cores and the `volatile` line · [guide 02](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/02-soc-memory-smp.md)

## Contract

A pose is a POD in `firmware/main/`: timestamp in microseconds, a camera, and a short list of instance transforms (translation plus a quaternion is enough). No pointers into sim memory. No ESP-IDF types.

Three slots, sized in `budget.md`. The publisher:

1. Writes a free slot.
2. Stores the slot index with an atomic release (or a seqlock). `volatile` is not the barrier.

The renderer, once per frame, on core 1:

1. Loads the latest published slot with an atomic acquire.
2. Keeps the previous pose as well.
3. Interpolates to the **frame start**, not to “whenever the sim last ran.” Normalize the quaternion (nlerp is enough). Lerp the translation. Clamp the blend to the newest pose.
4. If the newest timestamp is older than the glass cap (**12500 µs**), draw that pose as-is. Do not extrapolate.

The publisher never waits for present. The renderer never waits for the publisher, for I2C, or for Wi-Fi.

## Who runs where

| Work | Core |
| :--- | :--- |
| Present and interpolate | 1, pinned, no other task |
| Dummy publisher, and later the real sims | 0 |
| Wi-Fi task | 0 (task 2) |
| I2C | 0, one owner. This dummy publisher does not touch GPIO 41/42. |

The sim’s fake FreeRTOS is a `Sleep`. It is not SMP. Prove the mailbox with a host test: a writer thread and a reader thread, or a single-threaded script that publishes three poses and checks the reader never observes a torn transform (write a sentinel word first and last; they must match).

## Hitch test

Writer sleeps in irregular bursts (tens of milliseconds, sometimes longer than the glass cap). Reader logs present-start timestamps.

- Start-to-start stays on the task 3 cap while the frame is cheaper than 12500 µs. The publisher’s sleeps do not add to the gap. The Windows thread clock will wander; record it. On the chip, that gap stays within **2 ms** of 12500 µs while the writer is bursting.
- When the frame’s own work is longer than the cap, the gap is that work time.
- During a stall longer than the glass cap, the interpolated pose stops changing.
- When the writer resumes, the picture eases between the last held pose and the new one. It does not jump to a partial write.

Put the observed gaps in `budget.md` as a note under the glass-cap line.

## Done when

- A torn pose is rejected by the host test.
- A stall holds the last complete pose.
- Present starts stay on the glass cap while the publisher hitches and the frame is cheaper than that cap.
- The raster core still has no ESP-IDF include. Atomics from `stdatomic.h` are fine.

## Leave for later

No real IMU camera, no mesh file format. Task 6 consumes this pose.
