# 05 — Fusion, gravity, and IMU events

**Objective:** calculate a unit quaternion `q_device_to_world` at 100 Hz, where world +Y is opposite to gravity. Also calculate `jerk` and sparse `imu_evt` events for the bounce. **The screen camera is not this quaternion.** A clip can set `full_frame` and move the camera. Tilt cannot.

This is not a chip. It is the software of the IMU. Architecture §1 is the specification.

## Local papers

For the generic 6-axis procedure (without the habitat camera), refer to [esp32-s3 guide 05](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/05-6axis-filter.md). Paper: [madgwick_icorr2011.pdf](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/refs/fusion/madgwick_icorr2011.pdf). Read only the IMU chapters. Ignore MARG and the magnetometer.

Mahony TAC 2008 (an explicit complementary filter on SO(3), with gyro bias) is the other classic paper. It is behind the IEEE paywall: DOI [10.1109/TAC.2008.923738](https://doi.org/10.1109/TAC.2008.923738). You do **not** need it for v1. The Madgwick IMU filter and the procedure below are sufficient.

## Observability (learn this table)

| Movement | Filter | Habitat |
| :--- | :--- | :--- |
| Tilt, roll, put flat | The accelerometer gives **down**. The gyro makes it smooth. Stable. | The room VP **does not change**. Later, face-down → sleep. |
| Turn on the table | Only the gyro gives **yaw around gravity**. It drifts. | It must **not** turn the room. PLUS and double-tap are lizard one-shots. They do not recenter the camera. |
| Shake | High-pass jerk. | Impulse on the core spring (with cooldown). It does **not** move the camera, unless a clip sets `full_frame`. |

This board has no magnetometer. Do not invent a "soft iron" heading.

## Procedure that agrees with the architecture

Notation: body frame = chip; world +Y = up = −gravity.

For each FIFO sample `dt`, do these steps:

1. If `|a|` is near 1 g (not a hit), use `a` as gravity in the body frame.
2. Calculate the predicted gravity in the body frame from `q`. (Turn world `(0,1,0)` into the body frame, or use the opposite convention. Select one convention and do not change it.)
3. Error = `normalize(a) × predicted_down` (a small-angle vector in the body frame).
4. Correct the gyro: `ω' = ω_gyro - b - kp * error`. An optional `ki` term updates the bias `b`. Stop the `ki` term while `|a|` is not approximately 1 g.
5. Integrate with the quaternion kinematics `q̇ = ½ q ⊗ (0, ω')`. Then normalize.
6. The filter does **not** correct the yaw around world +Y. An optional debug recenter is permitted, but only for a horizon overlay.

This is a complementary filter (an explicit Mahony IMU filter). The Madgwick IMU `updateIMU` uses the same concept, with a gradient step instead of a cross product. At 100 Hz on an S3, the two filters are both satisfactory.

Classify the events **after** the filter:

- `jerk` more than the threshold, with cooldown → `imu_evt = shake`.
- A spike, then no movement → set-down.
- Face-down comes from `grav`.
- Held tilt is **off** in v1.

**Do not** send `q` to `look_at`. **Do not** add linear acceleration to gravity on each tick (no snow globe).

## Camera (published in the pose), not from `q`

```
view       = room.view          // authored 3/4, unless a clip overrides it
proj       = room.proj
full_frame = clip.full_frame    // shake or close-up; else 0
```

The window **does not move** until `full_frame` is set. The **bones** turn when the pet turns. Never use the IMU to turn the camera around the room.

Idle SPI: the interpolated pose is the same as the last presented pose. There is **no** `|Δq|` test.

## Open question (architecture §18)

Orthonormalize the IMU `up` against world +Y only if you draw a **debug** horizon. The habitat camera uses world +Y.

## Bring-up test (product test of step 4)

Use a live Nest slab. A pet is **not** necessary.

1. Tilt the board. The picture does not move, and Earth stays up.
2. Shake the board. A debug core jumps. After the cooldown, the row mask becomes empty.
3. Turn the board on the table. The filter yaw changes slowly. The room does not turn.

This test is the product. Do not start the skinned mesh until tilt does not turn the camera.
