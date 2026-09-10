# 05 — Fusion and the gravity-locked camera

**Goal:** a unit quaternion `q_device_to_world` at 100 Hz such that world +Y is opposite gravity, the screen is a boom camera, and **only** `view_idx` is discrete.

This is not a chip. It is the IMU’s software. Architecture §1 is the spec.

## Local paper

| File | Read |
| :--- | :--- |
| [`../fusion/madgwick_icorr2011.pdf`](../fusion/madgwick_icorr2011.pdf) | Sections on **IMU** (accel+gyro only), quaternion kinematics, gradient correction from gravity. Ignore MARG / magnetometer chapters |

Mahony TAC 2008 (explicit complementary filter on SO(3), gyro bias) is the other classic. It is IEEE-paywalled; DOI [10.1109/TAC.2008.923738](https://doi.org/10.1109/TAC.2008.923738). You do **not** need it to ship v1. Madgwick IMU + the recipe below is enough.

## Observability (memorize this)

| Motion | Filter |
| :--- | :--- |
| Tilt, roll, lay flat | Accel defines **down**. Gyro smooths. Solid. |
| Spin on the table | Gyro-only **yaw around gravity**. Drifts. PLUS / double-tap **recenter**. |
| Shake | High-pass jerk → flinch. **Not** the camera. |

No magnetometer on this board. Do not invent a “soft iron” heading.

## Recipe that matches architecture

Notation: body frame = chip; world +Y = up = −gravity.

Each FIFO sample `dt`:

1. If `|a|` is near 1 g (not a punch), treat `a` as gravity in body.
2. Predicted gravity in body from `q` (rotate world `(0,1,0)` into body, or the opposite convention — pick one and stick).
3. Error = `normalize(a) × predicted_down` (small-angle vector in body).
4. Correct gyro: `ω' = ω_gyro - b - kp * error` (optional `ki` on bias `b`, freeze `ki` while `|a|` is not ~1 g).
5. Integrate: quaternion kinematics `q̇ = ½ q ⊗ (0, ω')`, then normalize.
6. Yaw around world +Y is **not** corrected. Recenter zeros that component (PLUS, CST816 double-tap). Optional: decay yaw toward “front” while `|ω|` is tiny.

That is a complementary / explicit Mahony IMU filter. Madgwick’s IMU `updateIMU` is the same idea with a gradient step instead of a cross product; either is fine at 100 Hz on an S3.

**Do not** feed linear acceleration into the camera. Flinch uses `jerk` separately.

## Camera (Core 1, every 33 ms)

From the snapshot quaternion — **not** from a dodecahedron vertex:

```
forward = rotate(q, {0,0,-1})     // confirm on glass
up      = rotate(q, {0,1, 0})     // or orthonormalize to world +Y first (open question)
focus   = pet core
cam     = focus - forward * boom
// clamp elevation ≥ ~12°
view    = look_at(cam, focus, up)
```

Sheets:

```
dir_local = inv_R_core * normalize(cam - focus)
view_idx  = argmax(dot(dir_local, dodeca[i]))   // hysteresis ~0.02
```

Vertex 0 = +Y so a top-down hold gets a real top-down photo.

The window **orbits**. The photos **pop**. If pop is harsh: hysteresis, then maybe blend two sheets. Never snap `cam_pos`.

Idle SPI: `|Δq|` and `|Δcam|` below epsilon **and** springs settled **and** no clip. Quantized cameras got this for free; this one will not.

## Open question (architecture §18)

Raw IMU `up` dutch-angles the cube when you roll the phone. Orthonormalizing `up` against world +Y keeps the floor level (window on a room). **Try orthonormalize first in studio**; keep a compile-time switch.

## Bring-up test (step 4 product test)

Six room quads, live view/proj, **no pet**. Tilt the board: the cube stays glued to the earth, the window moves smoothly. Spin on the table: yaw creeps. Short-press PLUS: yaw snaps back. This test is the product. Do not start sprites until it feels right.
