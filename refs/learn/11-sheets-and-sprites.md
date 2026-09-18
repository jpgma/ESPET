# 11 — Sheets and sprites

← [10 cube](./10-gravity-locked-cube.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →

**Read:** [architecture 1 authored camera](../../architecture.md#authored-camera-discrete-yaw-sheets) · [architecture 10 Pet](../../architecture.md#10-rendering) · [architecture 11 Asset pipeline](../../architecture.md#11-asset-pipeline-pc)

**Code:** one L2 part, **4** idle yaw sheets (programmer art: 4 tinted rectangles with a marked hotspot). Blit at `project(pos)` with the **room** camera. `view_idx` from pet yaw vs `room.front`.

## Two cameras (not twenty)

| Camera | Job |
| :--- | :--- |
| **Room VP** | Authored `look_at`. Static per room. Backdrop + pivots. |
| **Bake cameras** | Four yaw headings at the **same** elevation. Index into an atlas. |

The window does not jump. The **photo** does, when the pet turns ~90°.

No [dodecahedron](../glossary.md#dodecahedron). No top-down sheet. [L0](../glossary.md#l0) (whole-pet blob) comes when you add a Hall; this lesson is Nest [L2](../glossary.md#l2). Architecture: the floor [shadow](../glossary.md#shadow) is a **separate** stamp, not in the blob; [occluders](../glossary.md#occluder) exist in S and L. Not this lesson.

## `view_idx`

```
view_idx = yaw_quad(pet_yaw - room.front)   /* 0..3 = F, R, B, L */
```

[Hysteresis](../glossary.md#hysteresis): switch only if the new winner beats the current by a small margin. Stops chatter on a boundary.

Until there is a walking core, `pet_yaw` is 0 (front sheet). A debug key can cycle yaw so you see all four.

## `SpriteRec` and blit

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;   /* pivot = attach */
} SpriteRec;
```

Pivot at `project(world(pos[i]))` with **room** `view`/`proj`. Colour-key index 0. **True billboard:** the quad faces the room camera. Do not also rotate it to a second bake heading.

Missing `sprite_id` = `0xFFFF` → idle clip, frame 0, same part and view.

One `view_idx` for all parts.

Paper-turn later: X-scale toward a line, swap sheet, expand. Not this lesson.

## Impostor

Bake heading vs a pet that is mid-yaw: the photo is the nearest 90°. Pivots are still correct 3D. **Do not** move the room camera to “fix” it.

If pop is harsh: hysteresis first, then optional 1-frame blend of two yaws (2× blit — measure).

## Bake contract (later art)

Blender: same FOV and elevation as runtime. For each clip frame and each of 4 yaws, hide other parts, crop alpha, record hotspot = attach. Exporter must fail if bake headings disagree with the runtime table by > 1e-5. First art: 1 idle × 4 yaws (this lesson).

## Sim gap

Still one thread. `view_idx` lives in the same loop as the room. When Core 0/1 split exists, `q` / `imu_evt` come from the seqlock; `view_idx` stays Core 1 and still does **not** use `q`.

## Checkpoint (sim)

Room stays put while you drag the IMU. A single “part” pops through 4 looks as you cycle pet yaw (debug key). Hotspot stuck to a projected point (debug crosshair). Overlay: `view_idx` (studio). Shake does not change `view_idx`.

## When the board arrives

Architecture §15 step 5: one L2 part, 4 idle sheets, blit at `project(pos)` while `view_idx` follows pet yaw. Room stays put; only the photo pops.

← [10 cube](./10-gravity-locked-cube.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →
