# 05 — Quaternions and rotation

← [04 vectors](./04-vectors-matrices-camera.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 06 First pixels](./06-first-pixels.md) →

**Read:** [architecture 1 (6-axis IMU)](../../architecture.md#6-axis-imu-sense-always-play-sparsely) · [guide 05](../guides/05-fusion-gravity-camera.md) · [madgwick_icorr2011.pdf](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/refs/fusion/madgwick_icorr2011.pdf) (IMU-only sections; skip magnetometer)

A [quaternion](../glossary.md#quaternion) `q = (x, y, z, w)` (architecture uses this field order) stores a 3D rotation in four numbers with **no gimbal lock**. ESPET’s Core 0 publishes `q_device_to_world` at 100 Hz.

## Why not Euler angles

[Yaw / pitch / roll](../glossary.md#euler-angles) as three angles: easy to print, terrible to integrate. At some orientations a degree of freedom disappears ([gimbal lock](../glossary.md#gimbal-lock)). Combining two poses is ugly. A unit quaternion composes with one multiply.

You may *display* Euler in a debug overlay. You do not *simulate* in Euler.

## Unit quaternion

`|q| = 1`. After every integrate, [normalize](../glossary.md#normalize). Numerical drift otherwise.

Rotate a vector `v` by `q`:

```
v' = q ⊗ (0,v) ⊗ q*
```

(`q*` = conjugate: negate x,y,z). You will write `quat_rotate(q, v)` once. Debug overlays may draw `forward`/`up`; the **room camera does not**.

Compose: `q_ab = q_a ⊗ q_b` (watch multiply order; pick one convention and comment it).

## Integrating gyro

The [gyro](../glossary.md#gyro) gives angular velocity `ω` in the device frame (rad/s). Over `dt`:

```
q̇ ≈ 0.5 * q ⊗ (0, ω)
q  += q̇ * dt
normalize(q)
```

That is the “gyro integrate” half of the [complementary filter](../glossary.md#complementary-filter). Alone, it **drifts** (bias, noise).

## Accel pulls down

The [accelerometer](../glossary.md#accelerometer) measures specific force. At rest, that is **gravity** in the device frame. You know world down is `-Y` (or equivalently world up is `+Y`).

Predicted gravity from `q` should match `normalize(accel)` when `|a| ≈ 1 g`. The mismatch is a small rotation error. Feed it back:

```
error = normalize(a) × predicted_down     /* body frame */
ω'    = ω_gyro - bias - kp * error
```

Then integrate `ω'`. Optional `ki` on bias; freeze it when `|a|` is not ~1 g (a punch is not gravity).

That is Mahony-style / Madgwick IMU `updateIMU`. Full recipe: [guide 05](../guides/05-fusion-gravity-camera.md). You code it in lesson 09.

## Observability (the product)

| What | Who sees it |
| :--- | :--- |
| Pitch and roll (tilt) | Accel (gravity) + gyro |
| Yaw around gravity | Gyro only. **Drifts.** |
| Heading like a compass | Nobody. No [magnetometer](../glossary.md#magnetometer). |

Spin the cube on the table: filter yaw creeps. That must **not** spin the habitat camera. PLUS / double-tap are lizard one-shots later, not camera recenter.

Shake: high-pass `|a|` → `jerk` → bounce / flinch. **Not** the camera.

## Device vs world

`q_device_to_world` rotates a vector in the chip frame into world. Use it for gravity, face-down, and `jerk`. Room `view` is authored (lesson 10). Sheets use pet yaw vs `room.front` — the room must not spin when the pet turns.

## Checkpoint

You can say out loud: “gravity gives me two axes; the third is dead reckoning.” If that sentence is fuzzy, re-read architecture’s IMU table before writing a filter.

## When the board arrives

Plot `|q|` (should stay ~1), `|a|` at rest (~1 g), and yaw while the toy sits still (should be quiet) vs while you spin it (should move and drift — that must not spin the room).

← [04 vectors](./04-vectors-matrices-camera.md) · [next: 06 First pixels](./06-first-pixels.md) →
