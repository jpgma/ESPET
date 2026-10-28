# 05 — Quaternions and rotation

← [04 vectors](./04-vectors-matrices-camera.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 06 First pixels](./06-first-pixels.md) →

**Read:** [architecture 1 (6-axis IMU)](../../architecture.md#6-axis-imu-sense-always-play-sparsely) · [guide 05](../guides/05-fusion-gravity-camera.md) · [madgwick_icorr2011.pdf](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/refs/fusion/madgwick_icorr2011.pdf) (only the IMU sections; do not read the magnetometer sections)

A [quaternion](../glossary.md#quaternion) `q = (x, y, z, w)` keeps a 3D rotation in four numbers, with **no gimbal lock**. (The architecture uses this field order.) Core 0 of ESPET publishes `q_device_to_world` at 100 Hz.

## Why not Euler angles

[Yaw, pitch and roll](../glossary.md#euler-angles) as three angles are easy to print, but very bad to integrate. At some orientations, one degree of freedom is lost ([gimbal lock](../glossary.md#gimbal-lock)). To combine two poses is difficult. A unit quaternion combines with one multiply.

You can *show* Euler angles in a debug overlay. Do not *simulate* in Euler angles.

## Unit quaternion

`|q| = 1`. After each integration, [normalize](../glossary.md#normalize). If you do not, the numbers drift.

To turn a vector `v` by `q`:

```
v' = q ⊗ (0,v) ⊗ q*
```

(`q*` is the conjugate: make x, y and z negative.) You will write `quat_rotate(q, v)` one time. Debug overlays can draw `forward` and `up`. The **room camera does not**.

Combine: `q_ab = q_a ⊗ q_b`. Be careful with the multiply order. Select one convention and write a comment about it.

## Gyro integration

The [gyro](../glossary.md#gyro) gives the angular velocity `ω` in the device frame (rad/s). For a time `dt`:

```
q̇ ≈ 0.5 * q ⊗ (0, ω)
q  += q̇ * dt
normalize(q)
```

This is the "gyro integration" half of the [complementary filter](../glossary.md#complementary-filter). Alone, it **drifts** (bias and noise).

## The accelerometer gives down

The [accelerometer](../glossary.md#accelerometer) measures specific force. At rest, that is **gravity** in the device frame. You know that world down is `-Y` (this is the same as world up is `+Y`).

When `|a| ≈ 1 g`, the predicted gravity from `q` must agree with `normalize(accel)`. The difference is a small rotation error. Send it back:

```
error = normalize(a) × predicted_down     /* body frame */
ω'    = ω_gyro - bias - kp * error
```

Then integrate `ω'`. An optional `ki` term updates the bias. Stop it when `|a|` is not approximately 1 g (a hit is not gravity).

This is the Mahony method, or the Madgwick IMU `updateIMU`. For the full procedure, refer to [guide 05](../guides/05-fusion-gravity-camera.md). You write the code in lesson 09.

## Observability (the product)

| Value | Source |
| :--- | :--- |
| Pitch and roll (tilt) | Accelerometer (gravity) and gyro |
| Yaw around gravity | Gyro only. **It drifts.** |
| Compass heading | None. There is no [magnetometer](../glossary.md#magnetometer). |

Turn the cube on the table: the filter yaw changes slowly. That must **not** turn the habitat camera. Later, PLUS and double-tap are lizard one-shots. They do not recenter the camera.

Shake: a high-pass filter on `|a|` → `jerk` → bounce or flinch. It does **not** move the camera.

## Device and world

`q_device_to_world` turns a vector from the chip frame into the world frame. Use it for gravity, face-down and `jerk`. The room `view` is set by the author (lesson 10). The sheets use the yaw of the pet against `room.front`. The room must not turn when the pet turns.

## Checkpoint

You can say this sentence aloud: "Gravity gives me two axes. The third axis is dead reckoning." If that sentence is not clear, read the IMU table of the architecture again before you write a filter.

## When the board arrives

Plot these values:

- `|q|`. It must stay approximately 1.
- `|a|` at rest. It must be approximately 1 g.
- The yaw when the toy does not move (it must be quiet), and the yaw when you turn the toy (it must move and drift, but the room must not turn).

← [04 vectors](./04-vectors-matrices-camera.md) · [next: 06 First pixels](./06-first-pixels.md) →
