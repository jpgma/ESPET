# 10 — Gravity-locked cube

← [09 filter](09-complementary-filter.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 11 Sheets](11-sheets-and-sprites.md) →

**Read:** [architecture 1](../../architecture.md#1-gravity-locked-cube-continuous-camera) · [architecture 10 Room](../../architecture.md#10-rendering) · [guide 05](../guides/05-fusion-gravity-camera.md) · lesson [04](04-vectors-matrices-camera.md)

**Code:** room raster in `firmware/` (e.g. `room.c`). **No pet.** Six quads, live `view`/`proj`, indexed blit + dirty rect from lesson 07.

This is the product test. If the cube leans with the glass, stop and fix the camera. Do not start sprites.

## World

Cube glued to Earth. Floor XZ, `+Y` up. Pet would stand on `Y=0` later. You draw floor, four walls, optional ceiling. **Skip the near face** (the glass).

Vertex colours or a tiny checker. Same 32-colour palette.

## Camera every frame

From snapshot `q` (not from a dodecahedron vertex). Helpers: [`look_at`](../glossary.md#look_at), [boom](../glossary.md#boom-camera).

```
forward = rotate(q, {0,0,-1})
up      = rotate(q, {0,1,0})   /* or orthonormalize to world +Y */
focus   = cube center for now  /* later: pet core; architecture lock is pet core */
cam     = focus - forward * boom
/* clamp elevation ≥ ~12° */
view    = look_at(cam, focus, up)
proj    = perspective(~55°, 1, near, far)
```

Use a dummy focus at cube centre until there is a core part. When the pet exists, switch focus without snapping `cam` to a vertex.

## Raster

For each quad: transform vertices, clip if you must (simple near-plane discard is enough at v1), fill triangles or scanlines in the indexed buffer. This is not a GPU. Keep it dumb and bounded.

Dirty rect: union of projected quad AABBs, or the whole 240×240 until that is too slow. Goal: when idle, skip SPI.

## Deadband (idle SPI)

A live IMU twitches. Skip frame if:

- `|Δq|` small, and
- `|Δcam|` small, and
- no clip (none yet), springs settled (none yet).

Log skipped vs drawn. If you draw 30 Hz while the window sits still, the deadband is too tight or you compared raw noise.

## What you must not do

- Snap `cam_pos` to a dodecahedron vertex to “save SPI” or “fix” impostor error.
- Rotate the room with device roll as if the cube were glued to the phone. World up stays world up.

## Checkpoint (sim)

Drag pitch/roll: you look *into* a room that stays level with the table. Spin: view yaws and may drift; recenter brings “front” back. Let go: after a moment, GRAM-hold (log hold). Elevation clamp: you cannot look up through the floor.

## When the board arrives

Architecture §15 step 4: complementary filter → **smooth cube**. Hold the board. Pick orthonormal-up vs raw `up` with the compile-time switch. Confirm `forward`.

← [09 filter](09-complementary-filter.md) · [next: 11 Sheets](11-sheets-and-sprites.md) →
