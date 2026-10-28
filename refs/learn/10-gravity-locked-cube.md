# 10 — Gravity-locked cube

← [09 filter](./09-complementary-filter.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 11 Meshes](./11-sheets-and-sprites.md) →

**Read:** [architecture 1](../../architecture.md#1-gravity-locked-habitat-authored-camera) · [architecture 10 Room](../../architecture.md#10-rendering) · [guide 05](../guides/05-fusion-gravity-camera.md) · lesson [04](./04-vectors-matrices-camera.md)

**Code:** a room in `firmware/` (for example, `room.c`). **No pet.** Use a 3/4 camera that the author sets, at Nest size. Raster a slab and a small number of walls as live triangles into the indexed framebuffer. Use the dirty rows from lesson 07.

This is the product test. If the room **turns** when you tilt the board, stop. The camera must not come from the IMU `q`. The cube stays attached to the Earth. Do not start the pet mesh.

## World

The cube is attached to the Earth. The floor is on XZ, and `+Y` is up. Later, the pet will stand on `Y=0`. Show a floor slab and two or three walls from a high **front** edge. If you draw a sky, use a small number of flat horizontal quads.

Programmer art: a small number of large triangles, with flat colors from the room [palette](../glossary.md#palette). Not a checker photograph. Not a texture. The indexed frame stays in DRAM. There is no backdrop to put in PSRAM.

## Camera, one time for each room

Helpers: [`look_at`](../glossary.md#look_at). **Not** a [boom from `q`](../glossary.md#boom-camera).

```
focus   = cube center          /* S rooms later: pet core */
eye     = high on the front face, looking slightly down   /* elev ~40° */
up      = {0,1,0}
view    = look_at(eye, focus, up)
proj    = perspective(~55°, 1, near, far)
```

Write one Nest S view in the code. Do not calculate it again from `q` on each frame.

The IMU continues to run (lesson 09). Later, use `jerk` for the bounce. Use `q` only for face-down and for a debug horizon.

## Raster (the live room)

Fill the slab and the walls with the same triangle path that lesson 11 will use for the pet. When the camera does not move, you can keep those screen-space triangles and skip the transform. But the pixels must be the same as a redraw. Do not blit a precomposed photograph.

This is not a GPU. **Do not make the view again from the IMU.** Later, a clip can set `full_frame` and move the camera on purpose. Tilt must not.

Dirty rows: all of the glass until you have a pose match. The goal: when the pose is the same as the last presented pose, the row mask is empty and you skip SPI.

## Deadband (idle SPI)

If the pose is the same (nothing moved), skip SPI. Do **not** test `|Δq|`. The author sets the window. Refer to the [architecture](../../architecture.md#4-core-allocation).

Record the skipped frames and the drawn frames. If you draw again on each frame while the board does not move, the IMU goes into the camera.

## Shake (preview)

If `jerk` is more than a threshold, record `imu_evt = shake`. Optionally, make a debug square bounce in Y, and then become stable. The cooldown is approximately 200 ms. This is the remaining IMU toy, not a camera. A camera shake is a `full_frame` clip, later.

## Prohibited actions

- Do not make `view` from `rotate(q, …)`.
- Do not turn the room with the device roll. World up stays world up.
- Do not snap anything to a dodecahedron.
- Do not paint the room into a 240×240 image and restore it.

## Checkpoint (simulator)

- Drag in pitch or roll: the **picture does not move**. The cube does not lean.
- Shake (fast drag): an optional debug mass jumps. Then the mask becomes empty.
- Turn: the filter yaw can drift. That must not turn the room.
- Face-down can wait for lesson 16.

## When the board arrives

Architecture §15 step 4: complementary filter → gravity and `jerk`. A live Nest slab. Hold the board: tilt does not turn the camera, and a shake makes a debug mass jump.

← [09 filter](./09-complementary-filter.md) · [next: 11 Meshes](./11-sheets-and-sprites.md) →
