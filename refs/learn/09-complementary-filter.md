# 09 — Complementary filter

← [08 IMU](./08-imu-registers.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 10 Cube](./10-gravity-locked-cube.md) →

**Read:** [guide 05](../guides/05-fusion-gravity-camera.md) · lesson [05 Quaternions](./05-quaternions.md) · [madgwick_icorr2011.pdf](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/refs/fusion/madgwick_icorr2011.pdf) IMU chapters · [architecture 1](../../architecture.md#1-gravity-locked-habitat-authored-camera)

**Code:** `firmware/filter.c` (or similar). Input: accel (g), gyro (rad/s), `dt`. Output: unit `q_device_to_world`, `grav[]`, `jerk`. Publish into a `SharedSnap`-shaped struct even if only one thread reads it.

## The two sensors

- Gyro: good for fast rotation, drifts.
- Accel: good for “where is down” when `|a| ≈ 1 g`, bad during punches.

[Complementary](../glossary.md#complementary-filter): trust gyro over short `dt`, pull toward accel’s down over time.

## Algorithm (copy from guide 05, then type it yourself)

Each sample:

1. If `|a|` near 1 g, treat `a` as gravity in device frame.
2. `predicted_down = rotate_inverse(q, world_down)` (world down = `(0,-1,0)` if up is `+Y`).
3. `error = normalize(a) × predicted_down`.
4. `ω' = ω - b - kp * error`. Optional `b += ki * error * dt`; freeze `ki` when `|a|` is wild.
5. Integrate quaternion with `ω'`, normalize.

Start `kp` small (try 0.5–2). Too large: accel noise on `q`. Too small: slow to find down after a flip.

**Yaw around gravity is not in `error`.** Filter yaw still drifts; that is fine — the room camera does not use `q`. Keep a debug recenter if you draw a horizon overlay. PLUS is not camera recenter.

`jerk`: high-pass `|a|` or `|Δa|/dt`. Publish it. Classify shake / set-down later. Do not feed it into `q`. Do not feed `q` into the room VP.

## dt

Sim: whatever time you actually waited (33 ms if you still sample in the display loop — **wrong rate**). Better: run the filter on every IMU read. If you still poll in the 30 Hz loop, `dt = 0.033` and the filter will feel sluggish. Split a 100 Hz-ish loop even on one thread (`dt = 0.01`) and let display stay 30 Hz. Architecture wants 100 Hz on Core 0.

## Debug overlay (studio only)

Print: `q` as four floats, `|q|-1` (near 0), yaw/pitch/roll for humans, `|a|`. Architecture suggestion: view index later, `CCOUNT` of blit — not this lesson.

## Checkpoint (sim)

- Device “flat” (sim rest): quaternion stable, pitch/roll near the orientation you mapped in lesson 08.
- Drag pitch/roll: `q` follows, settles when you stop (accel pull).
- Drag spin (yaw): `q` yaw moves and **keeps creeping** if you add a little bias. That must not spin a room (lesson 10).
- Shake (fast drag): `jerk` spikes; `q` does not explode if you gate `ki` / ignore accel when `|a|` is off 1 g.

No cube yet. You may keep the tinted fill.

## When the board arrives

Architecture §15 step 4 **product test** is lesson 10. This lesson on silicon: same prints, 100 Hz, no FIFO overflow.

← [08 IMU](./08-imu-registers.md) · [next: 10 Cube](./10-gravity-locked-cube.md) →
