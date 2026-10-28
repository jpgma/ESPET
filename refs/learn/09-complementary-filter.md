# 09 — Complementary filter

← [08 IMU](./08-imu-registers.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 10 Cube](./10-gravity-locked-cube.md) →

**Read:** [guide 05](../guides/05-fusion-gravity-camera.md) · lesson [05 Quaternions](./05-quaternions.md) · [madgwick_icorr2011.pdf](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/refs/fusion/madgwick_icorr2011.pdf) (IMU chapters) · [architecture 1](../../architecture.md#1-gravity-locked-habitat-authored-camera)

**Code:** `firmware/filter.c` (or a similar file). Input: accelerometer (g), gyro (rad/s), `dt`. Output: unit `q_device_to_world`, `grav[]`, `jerk`. Publish into a struct with the shape of `SharedSnap`, also if only one thread reads it.

## The two sensors

- Gyro: good for fast rotation, but it drifts.
- Accelerometer: good to find "down" when `|a| ≈ 1 g`, but bad during hits.

[Complementary filter](../glossary.md#complementary-filter): trust the gyro for a short `dt`. Over time, move toward the down of the accelerometer.

## Algorithm (copy it from guide 05, then type it yourself)

For each sample:

1. If `|a|` is near 1 g, use `a` as gravity in the device frame.
2. `predicted_down = rotate_inverse(q, world_down)` (world down = `(0,-1,0)` if up is `+Y`).
3. `error = normalize(a) × predicted_down`.
4. `ω' = ω - b - kp * error`. An optional term is `b += ki * error * dt`. Stop `ki` when `|a|` changes too much.
5. Integrate the quaternion with `ω'`. Normalize.

Start with a small `kp` (try 0.5–2). If `kp` is too large, the accelerometer noise goes into `q`. If `kp` is too small, the filter finds down slowly after a flip.

**The yaw around gravity is not in `error`.** The filter yaw still drifts. This is satisfactory, because the room camera does not use `q`. If you draw a horizon overlay, keep a debug recenter. PLUS does not recenter the camera.

`jerk`: a high-pass filter on `|a|`, or `|Δa|/dt`. Publish it. Classify shake and set-down later. Do not send it into `q`. Do not send `q` into the room VP.

## dt

On the simulator, `dt` is the time that you actually waited. If you still sample in the display loop, this is the present period, and that is the **wrong rate**. It is better to run the filter on each IMU read. If you still poll in the present loop, `dt` is the frame time, and the filter will be slow. Use a separate loop of approximately 100 Hz, also on one thread (`dt = 0.01`). Let the display run at the glass limit. The architecture specifies 100 Hz on Core 0.

## Debug overlay (studio only)

Print `q` as four floats, `|q|-1` (near 0), yaw/pitch/roll for people, and `|a|`. The architecture recommends the view index and the `CCOUNT` of the blit later. They are not in this lesson.

## Checkpoint (simulator)

- Device "flat" (simulator at rest): the quaternion is stable. Pitch and roll are near the orientation that you mapped in lesson 08.
- Drag in pitch or roll: `q` follows. When you stop, it becomes stable (the accelerometer pulls it).
- Drag to turn (yaw): the `q` yaw moves. If you add a small bias, it **continues to change**. That must not turn a room (lesson 10).
- Shake (fast drag): `jerk` has a spike. `q` does not become unstable if you stop `ki`, or ignore the accelerometer when `|a|` is not 1 g.

There is no cube yet. You can keep the tinted fill.

## When the board arrives

The **product test** of architecture §15 step 4 is lesson 10. On silicon, this lesson gives the same prints at 100 Hz, with no FIFO overflow.

← [08 IMU](./08-imu-registers.md) · [next: 10 Cube](./10-gravity-locked-cube.md) →
