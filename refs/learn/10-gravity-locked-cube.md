# 10 — Gravity-locked cube

← [09 filter](./09-complementary-filter.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 11 Meshes](./11-sheets-and-sprites.md) →

**Read:** [architecture 1](../../architecture.md#1-gravity-locked-habitat-authored-camera) · [architecture 10 Room](../../architecture.md#10-rendering) · [guide 05](../guides/05-fusion-gravity-camera.md) · lesson [04](./04-vectors-matrices-camera.md)

**Code:** room in `firmware/` (e.g. `room.c`). **No pet.** Authored Nest-sized 3/4 camera. Raster a slab and a few walls as live triangles into the indexed framebuffer. Dirty rows from lesson 07.

This is the product test. If the room **orbits** when you tilt the board, stop — the camera must not come from IMU `q`. The cube stays glued to Earth. Do not start the pet mesh.

## World

Cube glued to Earth. Floor XZ, `+Y` up. The pet will stand on `Y=0` later. You show a floor slab and two or three walls from a high **front** edge. Sky, if you draw it, is a few flat horizontal quads.

Programmer art: a handful of large triangles, flat colors from the room [palette](../glossary.md#palette). Not a checker photograph. Not a texture. The indexed frame stays in DRAM. There is no backdrop to put in PSRAM.

## Camera once per room

Helpers: [`look_at`](../glossary.md#look_at). **Not** [boom-from-`q`](../glossary.md#boom-camera).

```
focus   = cube center          /* S rooms later: pet core */
eye     = high on the front face, looking slightly down   /* elev ~40° */
up      = {0,1,0}
view    = look_at(eye, focus, up)
proj    = perspective(~55°, 1, near, far)
```

Hard-code one Nest S view. Do not recompute from `q` every frame.

IMU still runs (lesson 09). Use `jerk` later for bounce. Use `q` for face-down / debug horizon only.

## Raster (the room, live)

Fill the slab and walls with the same triangle path lesson 11 will use for the pet. A still camera may keep those screen-space triangles and skip the transform, but the pixels must match a redraw. Do not blit a precomposed photograph.

This is not a GPU. **Do not rebuild the view from the IMU.** A later clip may set `full_frame` and move the camera on purpose. Tilt must not.

Dirty rows: the whole glass until you have a pose match. Goal: when the pose matches the last presented one, the row mask is empty and you skip SPI.

## Deadband (idle SPI)

Skip SPI if the pose matches (nothing moved). **Do not** test `|Δq|`. The window is authored. See [architecture](../../architecture.md#4-core-allocation).

Log skipped vs drawn. If you draw 30 Hz while the board sits still, you are sampling IMU into the camera.

## Shake (preview)

If `jerk` is above a threshold, log `imu_evt = shake` and optionally bounce a debug square in Y, then settle. Cooldown ~200 ms. This is the leftover IMU toy, not a camera. A camera shake is a `full_frame` clip, later.

## What you must not do

- Build `view` from `rotate(q, …)`.
- Rotate the room with device roll. World up stays world up.
- Snap anything to a dodecahedron.
- Paint the room into a 240×240 image and restore it.

## Checkpoint (sim)

Drag pitch/roll: the **picture stays put**. The cube does not lean. Shake (fast drag): optional debug mass hops, then the mask goes empty. Spin: filter yaw may drift — that must not spin the room. Face-down can wait for lesson 16.

## When the board arrives

Architecture §15 step 4: complementary filter → gravity / `jerk`. Live Nest slab. Hold the board: tilt does not orbit; a shake hops a debug mass.

← [09 filter](./09-complementary-filter.md) · [next: 11 Meshes](./11-sheets-and-sprites.md) →
