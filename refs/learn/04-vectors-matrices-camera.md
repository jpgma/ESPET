# 04 — Vectors, matrices, camera

← [03 tasks](./03-tasks-cores-timing.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 05 Quaternions](./05-quaternions.md) →

**Read:** [architecture 1. Gravity-locked habitat](../../architecture.md#1-gravity-locked-habitat-authored-camera) (the camera formulas) · [architecture 10. Rendering](../../architecture.md#10-rendering)

You will project points through a camera that **the room sets**. This lesson is the math. There are no pixels yet. Lesson 10 uses these functions.

## Vectors

A 3D [vector](../glossary.md#vector) is `(x, y, z)`.

- **Length:** `|v| = sqrt(x²+y²+z²)`.
- **[Normalize](../glossary.md#normalize):** `v / |v|` → length 1. Use it only for directions.
- **[Dot](../glossary.md#dot-product):** `a·b = axbx+ayby+azbz`. A positive value means the same hemisphere. `yaw_quad` uses this to select a **painter's draw-order** slot from four headings. It is not a photograph. N·L uses the same helper at triangle setup.
- **[Cross](../glossary.md#cross-product):** `a×b` is perpendicular to the two vectors. You need it for `look_at` (the camera axes) and for the IMU error `a × predicted_down`. A bone 3×4 uses the same multiply.

Write these helpers and keep them: `vec3_add`, `vec3_scale`, `vec3_dot`, `vec3_cross`, `vec3_norm`. When you start lesson 10, put them in a file such as `firmware/math3.c`. Until then, keep them in a notebook.

## Coordinate frames

One point can have different numbers in different frames:

| Frame | Meaning |
| :--- | :--- |
| **World** | The cube, attached to the Earth. `+Y` = up = opposite to gravity. The floor is on XZ. |
| **Body** | The pet. The core is at the origin, `+Y` is up, yaw is 0. The core spring is in this frame. Bone locals are relative to their parent. |
| **Device** | The physical chip and screen. The IMU measures in this frame. |
| **Room** | The 3/4 camera that the room sets. `front` is a yaw around world +Y. |

In the architecture, the core spring is in body space. One core 3×4 matrix (translation, yaw and squash) is the parent of the skeleton. FK writes six bone world matrices. The screen camera is **not** the device frame.

**Tilt does not change the world gravity.** The core spring uses world `-Y`. The pet does not lean with the glass. The window does not turn around the room, unless a clip sets `full_frame`.

## Matrices

A 3×3 [rotation](../glossary.md#rotation-matrix) `R` turns a vector: `v' = R v`. A 4×4 [view](../glossary.md#view-matrix) and [projection](../glossary.md#projection-matrix) are the usual graphics stack.

You do not need a linear-algebra library. A 4×4 multiply and the inverse of a rigid transform (transpose the rotation, reverse the translation) are sufficient. Skinning is a mix of two bone 3×4 matrices on a vertex at triangle setup. There are two weights. Their sum is 1. That mix is not a pixel shader.

## `look_at`

[architecture §1](../../architecture.md#1-gravity-locked-habitat-authored-camera):

```
view      = room.view     // authored 3/4; elev ~40°; azimuth = room.front
proj      = room.proj     // perspective ~55°, aspect 1.0
light_dir = room.light_dir
```

Also write `look_at`, so that a room record can keep eye, target and up instead of a baked matrix.

`look_at(eye, target, up)`:

1. `zaxis = normalize(eye - target)` or `target - eye`. This depends on the convention. Select one convention that agrees with your multiply order. **Do not mix them.**
2. `xaxis = normalize(cross(up, zaxis))`
3. `yaxis = cross(zaxis, xaxis)`
4. Put the axes into a view matrix.

**S rooms:** target = the pet core (the camera shows the creature). **L rooms:** target = the room center (the camera shows the cube). The IMU `q` is not an input.

`up` is world +Y (or a small room tilt that the **author sets**, not a tilt from roll). Do not tilt the habitat camera from the device movement.

## Perspective

`proj = perspective(fov ≈ 55°, aspect 1.0, near, far)`.

After `clip = proj * view * world_point`, do the [perspective divide](../glossary.md#perspective-divide): `ndc = clip.xyz / clip.w`. Then change the range `-1…1` to the pixels `0…239`.

`project` is that pipeline on a **skinned** vertex. First mix the bones. Then clip and fill the triangle. There is no posed silhouette in a sprite.

## AABB

An [AABB](../glossary.md#aabb) is a screen-space box: minimum and maximum x and y. The dirty rect is the union of the projected AABBs of the moving items, with a margin. You send that window through SPI, and the rasterizer fills it.

## The pet turns, the window does not

The camera does not move when the pet yaws. The **bones** move. The painter's order is from far to near, on the triangles that you draw. A close-up is a clip that moves the camera and sets `full_frame`. It is not a second mesh.

The FOV is approximately 55°, the same in Blender and at runtime. S rooms show the pet (approximately 80–100 px). L rooms show the room (the pet is approximately 24–32 px). The skinned mesh is the same in the two cases.

## Checkpoint

Draw this on paper, with the board on a table and the screen up: world `+Y` is up, gravity is `-Y`, and the camera looks into the room from a high front edge. You can say these three things:

- Why a turn of the toy on the table is a *yaw* of the **device**.
- Why that yaw drifts (lesson 05).
- Why that yaw does **not** turn the habitat camera.

## When the board arrives

Make sure that the authored Nest view looks 3/4 into the cube, with Earth up. If debug overlays use `rotate(q, …)` later, that is an IMU visualization. It is not the room VP.

← [03 tasks](./03-tasks-cores-timing.md) · [next: 05 Quaternions](./05-quaternions.md) →
