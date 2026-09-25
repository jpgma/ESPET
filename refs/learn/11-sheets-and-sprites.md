# 11 — Skinned mesh (and stamps)

← [10 cube](./10-gravity-locked-cube.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →

Filename stays `11-sheets-and-sprites.md` so old links work. The product look is **not** 4-yaw photos and **not** six rigid parts.

**Read:** [architecture 1 camera](../../architecture.md#authored-camera-optional-full-frame-clip) · [architecture 10](../../architecture.md#10-rendering) · [architecture 11](../../architecture.md#11-asset-pipeline-pc)

**Code:** one weighted pet mesh (programmer art: a handful of triangles, two bone influences on each vertex). Raster it with the **room** camera. A debug clip rotates one bone. The room from lesson 10 stays on the same raster path.

## One camera, one mesh

| Thing | Job |
| :--- | :--- |
| **Room VP** | Authored `look_at`. Static until a clip sets `full_frame`. |
| **Bone palette** | Six world 3×4s. Core 1 blends two of them onto each vertex at triangle setup. |

The window does not jump when the pet turns. The **bones** turn. There is no photo pop and no second mesh for Hall.

No [dodecahedron](../glossary.md#dodecahedron). No yaw atlas. Close vs far is the camera and the room size. The floor [shadow](../glossary.md#shadow) is a **separate** stamp, not in the mesh.

## `MeshRec` and skin

```c
typedef struct {
    uint16_t vtx_off;    /* xyz + mat_id + two bone ids + two weights */
    uint16_t idx_off;
    uint16_t n_vtx;
    uint16_t n_tri;
} MeshRec;

typedef struct {
    uint8_t ramp[3];     /* shadow, mid, lit — indices in 1–63 */
} MatRamp;
```

Per triangle: blend the two bone matrices onto each of the three vertices, then clip and fill **one triangle at a time**. No deformed vertex buffer. The filler writes a palette index. Colour-key index 0 is for stamps, not for opaque tris.

**N·L at triangle setup,** after the blend: `band = quantize(dot(face_n, room.light_dir), 3)` → write `mat.ramp[band]`. Do not light in the pixel loop. No textures. No z-buffer. The triangle stays one flat color even though the elbow bends.

Stamps (shadow and FX only):

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;
} SpriteRec;
```

## Not an impostor

The room is triangles, same as the pet. Do not export a 240×240 photograph. A pet mid-turn is the skin, not the nearest photo. **Do not** move the room camera to “fix” a pose. A close-up is a clip that sets `full_frame`.

## Bake contract (later art)

Blender: same FOV and elevation as runtime. One mesh, six bones, at most two influences per vertex, weights sum to 1. Clip frames write bone locals only. Exporter must fail on a third influence or a ramp outside 1–63. First art: one mesh and one debug bone rotation (this lesson).

## Sim gap

Still one thread. When the pose mailbox exists, Core 0 publishes bone matrices and Core 1 skins. `board-sim` never rasterizes a habitat — your firmware does, and the fake GRAM shows it.

## Checkpoint (sim)

Room stays put while you drag the IMU. A debug bone rotation bends the mesh (two weights, so the elbow softens). Overlay: bone id. Shake does not change the room camera. N·L bands stay flat inside each triangle.

## When the board arrives

Architecture §15 step 5: one weighted mesh, six bones, two influences, flat N·L. A debug clip turns a bone. The room stays put.

← [10 cube](./10-gravity-locked-cube.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →
