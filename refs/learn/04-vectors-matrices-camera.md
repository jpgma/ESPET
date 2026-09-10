# 04 — Vectors, matrices, camera

← [03 tasks](03-tasks-cores-timing.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 05 Quaternions](05-quaternions.md) →

**Read:** [architecture 1. Gravity-locked cube](../../architecture.md#1-gravity-locked-cube-continuous-camera) (the camera formulas) · [architecture 10. Rendering](../../architecture.md#10-rendering)

You will raster a room as **six quads** from a live camera. This lesson is the math. No pixels yet — lesson 10 uses these functions.

## Vectors

A 3D [vector](../glossary.md#vector) is `(x, y, z)`.

- **Length:** `|v| = sqrt(x²+y²+z²)`.
- **[Normalize](../glossary.md#normalize):** `v / |v|` → length 1. Directions only.
- **[Dot](../glossary.md#dot-product):** `a·b = axbx+ayby+azbz`. Positive = same hemisphere. `argmax(dot(dir, dodeca[i]))` picks a sheet.
- **[Cross](../glossary.md#cross-product):** `a×b` is perpendicular to both. You need it for `look_at` (camera axes) and for the IMU error `a × predicted_down`.

Write four helpers you will keep: `vec3_add`, `vec3_scale`, `vec3_dot`, `vec3_cross`, `vec3_norm`. Put them in something like `firmware/math3.c` when you start lesson 10, or a notebook until then.

## Coordinate frames

Same point, different numbers:

| Frame | Meaning |
| :--- | :--- |
| **World** | Cube glued to Earth. `+Y` = up = opposite gravity. Floor on XZ. |
| **Body** | Pet. Core at origin, `+Y` up, yaw 0. Springs live here. |
| **Device** | The physical chip/screen. IMU measures here. |

Architecture: simulate in body space. One 3×4 (translate + yaw) maps body → world for projection, core-vs-cube collision, and camera focus.

**Tilt does not change world gravity.** Springs always use world `-Y` mapped into body. The pet does not lean with the glass.

## Matrices

A 3×3 [rotation](../glossary.md#rotation-matrix) `R` turns a vector: `v' = R v`. A 4×4 [view](../glossary.md#view-matrix) / [projection](../glossary.md#projection-matrix) is the usual graphics stack.

You do not need a linear-algebra library. 4×4 multiply and invert-of-rigid-transform (transpose rotation, undo translation) are enough.

## `look_at`

[architecture §1](../../architecture.md#1-gravity-locked-cube-continuous-camera):

```
forward = rotate(q, {0,0,-1})     // into the screen; confirm on bring-up
focus   = pet core
cam_pos = focus - forward * boom
view    = look_at(cam_pos, focus, up)
```

`look_at(eye, target, up)`:

1. `zaxis = normalize(eye - target)` or `target - eye` depending on convention — pick one, match your multiply order, **do not mix**.
2. `xaxis = normalize(cross(up, zaxis))`
3. `yaxis = cross(zaxis, xaxis)`
4. Pack into a view matrix.

**Open question:** raw IMU `up` dutch-angles the cube when you roll. **Try orthonormalizing `up` against world +Y first** (floor stays level). Compile-time switch.

**Elevation clamp:** if the camera would look from below ~12°, lift it. No peek-under. `view_idx` only among vertices in that hemisphere.

## Perspective

`proj = perspective(fov ≈ 55°, aspect 1.0, near, far)`.

After `clip = proj * view * world_point`, [perspective divide](../glossary.md#perspective-divide): `ndc = clip.xyz / clip.w`. Then map `-1…1` to pixels `0…239`.

`project(pos)` for a part pivot = that pipeline. The sprite hangs from that hotspot. The posed silhouette is **in the pixels**, not a second transform.

## AABB

An [AABB](../glossary.md#aabb) is a screen-space box: min/max x, y. Dirty rect = union of projected part AABBs + margin. That window is what you send over SPI.

## Boom

FOV ~55°, pet ~1/3 of the frame, camera outside the cube. Same FOV/boom in Blender so sheet scale is in the ballpark. Off-vertex they will not match pixel-perfect. That mismatch is accepted.

## Checkpoint

On paper, with the board screen-up on a table: world `+Y` up, gravity `-Y`, camera looking at the pet core. You can say why spinning the toy on the table is *yaw* and why that will drift (lesson 05).

## When the board arrives

Confirm `forward = rotate(q, {0,0,-1})` really is “into the glass.” If the cube yaws the wrong way, negate one axis here, not in the filter.

← [03 tasks](03-tasks-cores-timing.md) · [next: 05 Quaternions](05-quaternions.md) →
