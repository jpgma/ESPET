# 05 — Fusion, gravity, and IMU events

**Goal:** a unit quaternion `q_device_to_world` at 100 Hz such that world +Y is opposite gravity, plus `jerk` / sparse `imu_evt` for bounce. The **screen camera is not this quaternion.** `view_idx` is pet yaw vs `room.front` (0..3).

This is not a chip. It is the IMU’s software. Architecture §1 is the spec.

## Local paper

| File | Read |
| :--- | :--- |
| [`../fusion/madgwick_icorr2011.pdf`](../fusion/madgwick_icorr2011.pdf) | Sections on **IMU** (accel+gyro only), quaternion kinematics, gradient correction from gravity. Ignore MARG / magnetometer chapters |

Mahony TAC 2008 (explicit complementary filter on SO(3), gyro bias) is the other classic. It is IEEE-paywalled; DOI [10.1109/TAC.2008.923738](https://doi.org/10.1109/TAC.2008.923738). You do **not** need it to ship v1. Madgwick IMU + the recipe below is enough.

## Observability (memorize this)

| Motion | Filter | Habitat |
| :--- | :--- | :--- |
| Tilt, roll, lay flat | Accel defines **down**. Gyro smooths. Solid. | Room VP **unchanged**. Face-down later → sleep. |
| Spin on the table | Gyro-only **yaw around gravity**. Drifts. | Must **not** spin the room. PLUS / double-tap are lizard one-shots, not camera recenter. |
| Shake | High-pass jerk. | Impulse on `vel[]` (cooldown). **Not** the camera. |

No magnetometer on this board. Do not invent a “soft iron” heading.

## Recipe that matches architecture

Notation: body frame = chip; world +Y = up = −gravity.

Each FIFO sample `dt`:

1. If `|a|` is near 1 g (not a punch), treat `a` as gravity in body.
2. Predicted gravity in body from `q` (rotate world `(0,1,0)` into body, or the opposite convention — pick one and stick).
3. Error = `normalize(a) × predicted_down` (small-angle vector in body).
4. Correct gyro: `ω' = ω_gyro - b - kp * error` (optional `ki` on bias `b`, freeze `ki` while `|a|` is not ~1 g).
5. Integrate: quaternion kinematics `q̇ = ½ q ⊗ (0, ω')`, then normalize.
6. Yaw around world +Y is **not** corrected. Optional debug recenter for a horizon overlay only.

That is a complementary / explicit Mahony IMU filter. Madgwick’s IMU `updateIMU` is the same idea with a gradient step instead of a cross product; either is fine at 100 Hz on an S3.

Classify **after** the filter: `jerk` above threshold + cooldown → `imu_evt = shake`. Spike-then-still → set-down. Face-down from `grav`. Held tilt **off** in v1.

**Do not** feed `q` into Core 1 `look_at`. **Do not** feed linear acceleration into gravity every tick (no snow-globe).

## Camera (Core 1, every 33 ms) — not from `q`

```
view     = room.view                 // authored 3/4
proj     = room.proj
view_idx = yaw_quad(pet_yaw - room.front)   // 0..3, hysteresis
lod      = room.lod                  // L0 or L2
```

The window **stays**. The photos **pop** when the pet turns. If pop is harsh: hysteresis, then maybe blend two yaws. Never IMU-orbit `cam_pos`.

Idle SPI: springs settled, no clip, toys settled, `fx_live==0`. **No** `|Δq|` / `|Δcam|` test.

## Open question (architecture §18)

Orthonormalize IMU `up` against world +Y only if you draw a **debug** horizon. The habitat camera is authored with world +Y.

## Bring-up test (step 4 product test)

Authored Nest backdrop, **no pet** required. Tilt the board: the picture stays put, Earth-up. Shake: a debug mass hops, cooldown, then GRAM-hold. Spin on the table: filter yaw creeps; the room does not. This test is the product. Do not start sprites until tilt-does-not-orbit is true.
