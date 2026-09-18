# 10 — Gravity-locked cube

← [09 filter](./09-complementary-filter.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 11 Sheets](./11-sheets-and-sprites.md) →

**Read:** [architecture 1](../../architecture.md#1-gravity-locked-habitat-authored-camera) · [architecture 10 Room](../../architecture.md#10-rendering) · [guide 05](../guides/05-fusion-gravity-camera.md) · lesson [04](./04-vectors-matrices-camera.md)

**Code:** room in `firmware/` (e.g. `room.c`). **No pet.** Authored Nest-sized 3/4 camera, indexed backdrop (or six quads rastered **once** into a 240×240 buffer), dirty rect from lesson 07.

This is the product test. If the room **orbits** when you tilt the board, stop — the camera must not come from IMU `q`. The cube stays glued to Earth. Do not start sprites.

## World

Cube glued to Earth. Floor XZ, `+Y` up. Pet would stand on `Y=0` later. You show floor, walls, optional ceiling from a high **front** edge.

Programmer art: fill a 240×240 indexed [backdrop](../glossary.md#backdrop) with a checker floor and three walls. Same 32-colour [palette](../glossary.md#palette) for this lesson (one table; per-room palettes come with Hall). Keep it in a buffer you will later put in PSRAM; for sim, DRAM is fine.

## Camera once per room

Helpers: [`look_at`](../glossary.md#look_at). **Not** [boom-from-`q`](../glossary.md#boom-camera).

```
focus   = cube center          /* S rooms later: pet core */
eye     = high on the front face, looking slightly down   /* elev ~40° */
up      = {0,1,0}
view    = look_at(eye, focus, up)
proj    = perspective(~55°, 1, near, far)
```

Bake or hard-code one Nest S view. Do not recompute from snapshot `q` every frame.

IMU still runs (lesson 09). Use `jerk` later for bounce. Use `q` for face-down / debug horizon only.

## Raster

Either:

- blit the precomposed backdrop into the indexed FB, or
- fill six quads **once** into that backdrop, then treat it as a bitmap.

This is not a GPU. Do not re-raster the room from a moving VP.

Dirty rect: whole 240×240 until something moves. Goal: when idle, skip SPI. Tilt must **not** dirty the room.

## Deadband (idle SPI)

Skip frame if nothing in the habitat moved (no clip, springs settled — none yet). **Do not** test `|Δq|` / `|Δcam|`. The window is authored. Later GRAM-hold also waits for toys settled and [`fx_live==0`](../glossary.md#fx) ([architecture](../../architecture.md#4-core-allocation)).

Log skipped vs drawn. If you draw 30 Hz while the board sits still, you are sampling IMU into the camera.

## Shake (preview)

If `jerk` is above a threshold, log `imu_evt = shake` and optionally bounce a debug square in Y, then settle. Cooldown ~200 ms. This is the leftover IMU toy, not a camera.

## What you must not do

- Build `view` from `rotate(q, …)`.
- Rotate the room with device roll as if the cube were glued to the phone. World up stays world up.
- Snap anything to a dodecahedron. That index is gone.

## Checkpoint (sim)

Drag pitch/roll: the **picture stays put**. The cube does not lean. Shake (fast drag): optional debug mass hops, then GRAM-hold. Spin: filter yaw may drift — that must not spin the room. Face-down can wait for lesson 16.

## When the board arrives

Architecture §15 step 4: complementary filter → gravity / `jerk`. Authored Nest backdrop. Hold the board: tilt does not orbit; a shake hops a debug mass.

← [09 filter](./09-complementary-filter.md) · [next: 11 Sheets](./11-sheets-and-sprites.md) →
