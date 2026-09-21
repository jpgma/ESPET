# 11 — Rigid meshes (and stamps)

← [10 cube](./10-gravity-locked-cube.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →

Filename stays `11-sheets-and-sprites.md` so old links work. The product look is **not** 4-yaw photos.

**Read:** [architecture 1 authored camera](../../architecture.md#authored-camera-discrete-draw-order) · [architecture 10 Pet](../../architecture.md#10-rendering) · [architecture 11 Asset pipeline](../../architecture.md#11-asset-pipeline-pc)

**Code:** one L2 part **mesh** (programmer art: a handful of vertex-colored tris, origin = attach). Raster at `project(pos)` with the **room** camera. Pet yaw is **continuous**. `view_idx` from pet yaw vs `room.front` is painter's **order** only — debug overlay, not a sheet picker.

## Two cameras (not twenty, not four pet bakes)

| Camera | Job |
| :--- | :--- |
| **Room VP** | Authored `look_at`. Static per room. Backdrop + projection. |
| **Part 3×4** | Translate to `world(pos[i])`, aim bind +Y along attach → `pos`/`tip`. Continuous yaw. |

The window does not jump. The **mesh rotates** when the pet turns. There is no photo pop.

No [dodecahedron](../glossary.md#dodecahedron). No yaw atlas. [L0](../glossary.md#l0) (one combined mesh) comes when you add a Hall; this lesson is Nest [L2](../glossary.md#l2). Architecture: the floor [shadow](../glossary.md#shadow) is a **separate** stamp, not in the mesh; [occluders](../glossary.md#occluder) are static meshes in S and L. Not this lesson.

## `view_idx` (order, not appearance)

```
view_idx = yaw_quad(pet_yaw - room.front)   /* 0..3 = F, R, B, L — draw_order only */
```

[Hysteresis](../glossary.md#hysteresis): switch only if the new winner beats the current by a small margin. Stops painter's-order chatter on a boundary.

Until there is a walking core, `pet_yaw` can be a debug key. Cycle it: the mesh turns smoothly; `view_idx` may tick 0..3 in the overlay.

## `MeshRec` and raster

```c
typedef struct {
    uint16_t vtx_off;    /* packed xyz + mat_id */
    uint16_t idx_off;
    uint16_t n_vtx;
    uint16_t n_tri;
} MeshRec;

typedef struct {
    uint8_t ramp[3];     /* shadow, mid, lit — indices in 1–15 */
} MatRamp;
```

Origin at attach. Clip + project + scanline **one triangle at a time** into the dirty window of the indexed FB. Colour-key index 0 is for stamps (shadow / FX later), not for opaque part tris.

**N·L at triangle setup:** `band = quantize(dot(face_n, room.light_dir), 3)` → write `mat.ramp[band]`. Do not light in the pixel loop. No textures. No z-buffer.

Stamps (not this lesson, but the type you will reuse):

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;
} SpriteRec;   /* shadow + FX only */
```

## Not an impostor

Bake cameras exist for **room backdrops**. Pet export is bind-pose meshes. A pet mid-yaw is the actual rotation, not the nearest 90° photo. **Do not** move the room camera to “fix” anything. **Do not** paper-turn or blend two sheets — those hacks are gone.

## Bake contract (later art)

Blender: same FOV and elevation as runtime **for the room photograph**. Pet: one bind-pose mesh per part, origin = attach, `mat_id` on verts/faces. Clip frames write `rest` only. Exporter must fail if ramps leave 1–15. First art: 1 idle part mesh (this lesson).

## Sim gap

Still one thread. `view_idx` lives in the same loop as the room. When Core 0/1 split exists, `q` / `imu_evt` come from the seqlock; `view_idx` stays Core 1 and still does **not** use `q`. `board-sim` never rasterizes a cube — your firmware does, and the fake GRAM shows it.

## Checkpoint (sim)

Room stays put while you drag the IMU. A single part mesh **rotates** as you cycle pet yaw (debug key). Origin stuck to a projected point (debug crosshair). Overlay: `view_idx` (studio, order only). Shake does not change the room camera. N·L bands visible when you spin the part.

## When the board arrives

Architecture §15 step 5: one L2 part mesh, continuous yaw, raster at `project(pos)` with N·L bands. Room stays put; the mesh turns with pet yaw.

← [10 cube](./10-gravity-locked-cube.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →
