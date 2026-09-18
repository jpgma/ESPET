# 04 — Vectors, matrices, camera

← [03 tasks](./03-tasks-cores-timing.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 05 Quaternions](./05-quaternions.md) →

**Read:** [architecture 1. Gravity-locked habitat](../../architecture.md#1-gravity-locked-habitat-authored-camera) (the camera formulas) · [architecture 10. Rendering](../../architecture.md#10-rendering)

You will project points through a **room-authored** camera. This lesson is the math. No pixels yet — lesson 10 uses these functions.

## Vectors

A 3D [vector](../glossary.md#vector) is `(x, y, z)`.

- **Length:** `|v| = sqrt(x²+y²+z²)`.
- **[Normalize](../glossary.md#normalize):** `v / |v|` → length 1. Directions only.
- **[Dot](../glossary.md#dot-product):** `a·b = axbx+ayby+azbz`. Positive = same hemisphere. `yaw_quad` picks a sheet from four headings this way.
- **[Cross](../glossary.md#cross-product):** `a×b` is perpendicular to both. You need it for `look_at` (camera axes) and for the IMU error `a × predicted_down`.

Write four helpers you will keep: `vec3_add`, `vec3_scale`, `vec3_dot`, `vec3_cross`, `vec3_norm`. Put them in something like `firmware/math3.c` when you start lesson 10, or a notebook until then.

## Coordinate frames

Same point, different numbers:

| Frame | Meaning |
| :--- | :--- |
| **World** | Cube glued to Earth. `+Y` = up = opposite gravity. Floor on XZ. |
| **Body** | Pet. Core at origin, `+Y` up, yaw 0. Springs live here. |
| **Device** | The physical chip/screen. IMU measures here. |
| **Room** | Authored 3/4 camera. `front` is a yaw around world +Y. |

Architecture: simulate in body space. One 3×4 (translate + yaw) maps body → world for projection and core-vs-room collision. The screen camera is **not** the device frame.

**Tilt does not change world gravity.** Springs always use world `-Y` mapped into body. The pet does not lean with the glass. The window does not orbit.

## Matrices

A 3×3 [rotation](../glossary.md#rotation-matrix) `R` turns a vector: `v' = R v`. A 4×4 [view](../glossary.md#view-matrix) / [projection](../glossary.md#projection-matrix) is the usual graphics stack.

You do not need a linear-algebra library. 4×4 multiply and invert-of-rigid-transform (transpose rotation, undo translation) are enough.

## `look_at`

[architecture §1](../../architecture.md#1-gravity-locked-habitat-authored-camera):

```
view = room.view     // authored 3/4; elev ~40°; azimuth = room.front
proj = room.proj     // perspective ~55°, aspect 1.0
```

You still write `look_at` so a room record can store eye / target / up instead of a baked matrix.

`look_at(eye, target, up)`:

1. `zaxis = normalize(eye - target)` or `target - eye` depending on convention — pick one, match your multiply order, **do not mix**.
2. `xaxis = normalize(cross(up, zaxis))`
3. `yaxis = cross(zaxis, xaxis)`
4. Pack into a view matrix.

**S rooms:** target = pet core (frame the creature). **L rooms:** target = room centre (frame the cube). IMU `q` is not an input.

`up` is world +Y (or a slight room tilt that is **authored**, not from roll). Do not dutch-angle the habitat from the phone.

## Perspective

`proj = perspective(fov ≈ 55°, aspect 1.0, near, far)`.

After `clip = proj * view * world_point`, [perspective divide](../glossary.md#perspective-divide): `ndc = clip.xyz / clip.w`. Then map `-1…1` to pixels `0…239`.

`project(pos)` for a part pivot = that pipeline. The sprite hangs from that hotspot. The posed silhouette is **in the pixels**, not a second transform.

## AABB

An [AABB](../glossary.md#aabb) is a screen-space box: min/max x, y. Dirty rect = union of projected moving AABBs + margin. That window is what you send over SPI.

## Yaw sheets

Four headings in XZ: front, right, back, left, relative to `room.front`. `view_idx = yaw_quad(pet_yaw - room.front)` with [hysteresis](../glossary.md#hysteresis). The camera does not move when the pet turns; the **photo** does.

FOV ~55°, same in Blender as runtime. S bake: pet ~1/3 of the frame. L bake: pet ~24–32 px.

## Checkpoint

On paper, with the board screen-up on a table: world `+Y` up, gravity `-Y`, camera looking into the room from a high front edge. You can say why spinning the toy on the table is *yaw* of the **device**, why that drifts (lesson 05), and why that yaw does **not** turn the habitat camera.

## When the board arrives

Confirm the authored Nest view looks 3/4 into the cube, Earth-up. If later debug overlays use `rotate(q, …)`, that is IMU visualization, not the room VP.

← [03 tasks](./03-tasks-cores-timing.md) · [next: 05 Quaternions](./05-quaternions.md) →
