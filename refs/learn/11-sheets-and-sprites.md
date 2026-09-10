# 11 — Sheets and sprites

← [10 cube](10-gravity-locked-cube.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 12 Springs](12-clips-springs-hitboxes.md) →

**Read:** [architecture 1 continuous vs discrete](../../architecture.md#continuous-camera-discrete-sheets) · [architecture 10 Pet](../../architecture.md#10-rendering) · [architecture 11 Asset pipeline](../../architecture.md#11-asset-pipeline-pc)

**Code:** one part, 20 idle sheets (can be programmer art: 20 tinted rectangles with a marked hotspot). Blit at `project(pos)` with live camera. `view_idx` from nearest dodecahedron vertex.

## Two cameras

| Camera | Job |
| :--- | :--- |
| **Live boom** | View/projection. Smooth. Room + pivots. |
| **Bake cameras** | 20 vertices of a [dodecahedron](../glossary.md#dodecahedron). Index into an atlas. **Not** where you put `cam_pos`. |

Neighbor vertices ~37° apart. The window does not jump. The **photo** does.

Vertex 0 = `+Y` (top-down sheet when you look down).

## `view_idx`

```
dir_world = normalize(cam_pos - focus)
dir_local = inv_R_core * dir_world     /* pet yaw cycles sheets, not the room */
view_idx  = argmax(dot(dir_local, dodeca[i]))
```

[Hysteresis](../glossary.md#hysteresis): switch only if the new winner beats the current by ~0.02 on the dot. Stops chatter on a boundary.

Hemisphere: only consider vertices above the ~12° clamp so a clamped camera does not pick a below-floor sheet. Bake all 20 anyway.

Until there is a walking core, `R_core` is identity.

## `SpriteRec` and blit

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;   /* pivot = attach */
} SpriteRec;
```

Pivot at `project(world(pos[i]))`. Colour-key index 0. **True billboard:** the quad faces the *live* camera. Do not also rotate it to the bake camera.

Missing `sprite_id` = `0xFFFF` → idle clip, frame 0, same part and view.

One `view_idx` for all parts.

## Impostor error (accepted)

Bake camera on a vertex; live camera between vertices (up to ~18°). Pivots are correct 3D. Pixels are a nearby photo. Parallax mismatch between parts is the cost of a smooth window. **Do not** snap `cam_pos` to fix it.

If pop is harsh: hysteresis first, then optional 1-frame blend of two sheets (2× blit — measure).

## Bake contract (later art)

Blender: same FOV/boom as runtime. For each clip frame and each vertex, hide other parts, crop alpha, record hotspot = attach. Exporter must fail if bake *index* cameras disagree with `dodeca_vertex[]` by > 1e-5. First art can be 1 idle × 20 views.

## Sim gap

Still one thread. `view_idx` can live in the same loop as the cube. When Core 0/1 split exists, `q` comes from the seqlock; `view_idx` stays Core 1.

## Checkpoint (sim)

Room still smooth. A single “part” pops through 20 looks as you orbit. Top-down hold → vertex 0 sheet. Hotspot stuck to a projected point in the cube (use a debug crosshair). Overlay: `view_idx` (studio).

## When the board arrives

Architecture §15 step 5: one part, 20 idle sheets, blit at live `project(pos)` while `view_idx` follows. Cube stays smooth; only the photo pops.

← [10 cube](10-gravity-locked-cube.md) · [next: 12 Springs](12-clips-springs-hitboxes.md) →
