# 06 — Mesh, clip, dirty rows

← [pose mailbox](./05-pose-mailbox.md) · [index](./README.md) · [next: painter order](./07-painter.md) →

**Goal:** a small scene becomes screen triangles in internal scratch, and a frame that did not move produces an empty row mask and no SPI.

**Depends on:** tasks 4 and 5.

**Read:** [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) GRAM-hold · [guide 05](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/05-6axis-filter.md) only for the yaw limit, if you hook a camera at all

## Scene

Hard-code one creature-sized mesh and a few props in a C table in `firmware/main/`. Stay under the **1024** triangle cap. This is not an asset pipeline.

Store the tables in ordinary const data. On the chip that may land in flash, which is fine: copy the triangles you will draw into the internal scratch from task 1 at the start of the frame. The filler from task 4 reads the scratch, not PSRAM and not the const mesh, while it is writing pixels.

## Transform and clip

Float or Q16 at the vertices is fine. The filler stays integer.

Per frame, from the interpolated pose:

1. Transform and project. Perspective divide once per vertex.
2. Clip to the 240×240 screen before raster. Drop triangles that miss the screen. Clip the ones that cross an edge; a six-vertex result split into triangles is enough.
3. Write screen-space triangles into the scratch, with a color index (flat, or a vertex index you resolve to one index for now) and a sort key for task 7 (a 16-bit depth of the triangle).
4. Build the row mask from each triangle’s Y range.

Sub-pixel motion, and a pose that matches the last presented pose, leaves the mask empty. Task 3 then skips SPI.

A flag on the pose, `full_frame`, sets all 240 rows. Ordinary frames leave it clear. Task 8 is what sets it. You can also set it when the camera moves by more than a pixel, which is the same idea.

## Camera

Default camera is fixed. If you attach the board’s tilt, use pitch and roll only. Yaw around gravity drifts; do not treat it as a compass. A moving camera is a full-frame event.

## Tests

- One known view: a handful of projected vertices checked against hand calculation (a cube corner is enough).
- A triangle that crosses `x = 0` is clipped; the filler never sees a negative X.
- Two identical poses in a row: the second present does not call `draw_bitmap`.
- Move one instance by several pixels: only the old and new Y spans are set in the mask.
- `full_frame` set: all 240 bits set, even if the meshes are small.

## Done when

Those five tests pass on the host, and `sim.bat` shows the still hold (the window does not flicker from a needless full clear) and the moved instance.

## Leave for later

Painter sort is task 7. The sort key is stored here and not used yet. No textures.
