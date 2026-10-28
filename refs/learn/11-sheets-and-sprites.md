# 11 — Skinned mesh (and stamps)

← [10 cube](./10-gravity-locked-cube.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →

The file name stays `11-sheets-and-sprites.md`, so that old links continue to operate. The product look is **not** photographs at 4 yaws, and it is **not** six rigid parts.

**Read:** [architecture 1 camera](../../architecture.md#authored-camera-optional-full-frame-clip) · [architecture 10](../../architecture.md#10-rendering) · [architecture 11](../../architecture.md#11-asset-pipeline-pc)

**Code:** one weighted pet mesh (programmer art: a small number of triangles, two bone influences on each vertex). Raster it with the **room** camera. A debug clip turns one bone. The room from lesson 10 stays on the same raster path.

## One camera, one mesh

| Item | Function |
| :--- | :--- |
| **Room VP** | A `look_at` that the author sets. It does not move until a clip sets `full_frame`. |
| **Bone palette** | Six world 3×4 matrices. At triangle setup, Core 1 mixes two of them on each vertex. |

The window does not jump when the pet turns. The **bones** turn. There is no sudden change of photograph and no second mesh for the Hall.

No [dodecahedron](../glossary.md#dodecahedron). No yaw atlas. The camera and the room size make the pet close or far. The floor [shadow](../glossary.md#shadow) is a **separate** stamp. It is not in the mesh.

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

For each triangle: mix the two bone matrices on each of the three vertices. Then clip and fill **one triangle at a time**. There is no deformed vertex buffer. The filler writes a palette index. The color-key index 0 is for stamps, not for opaque triangles.

**Calculate N·L at triangle setup,** after the mix: `band = quantize(dot(face_n, room.light_dir), 3)` → write `mat.ramp[band]`. Do not calculate the light in the pixel loop. No textures. No z-buffer. The triangle stays one flat color, also when the elbow bends.

Stamps (shadow and FX only):

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;
} SpriteRec;
```

## Not an impostor

The room is triangles, the same as the pet. Do not export a 240×240 photograph. A pet that is in a turn is the skin, not the nearest photograph. **Do not** move the room camera to "correct" a pose. A close-up is a clip that sets `full_frame`.

## Bake contract (later art)

Blender: the same FOV and elevation as the runtime. One mesh, six bones, a maximum of two influences for each vertex, and a weight sum of 1. The clip frames write only bone locals. The exporter must fail on a third influence or on a ramp out of 1–63. The first art is one mesh and one debug bone rotation (this lesson).

## Simulator gap

The simulator still has one thread. When the pose mailbox is available, Core 0 publishes the bone matrices and Core 1 does the skinning. `board-sim` never rasterizes a habitat. Your firmware does it, and the simulated GRAM shows it.

## Checkpoint (simulator)

- The room does not move while you drag the IMU.
- A debug bone rotation bends the mesh. (There are two weights, so the elbow is soft.)
- The overlay shows the bone ID.
- A shake does not change the room camera.
- The N·L bands stay flat in each triangle.

## When the board arrives

Architecture §15 step 5: one weighted mesh, six bones, two influences, flat N·L. A debug clip turns a bone. The room does not move.

← [10 cube](./10-gravity-locked-cube.md) · [next: 12 Springs](./12-clips-springs-hitboxes.md) →
