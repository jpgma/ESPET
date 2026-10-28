# 06 — Mesh, clip, dirty rows

← [pose mailbox](./05-pose-mailbox.md) · [index](./README.md) · [next: painter order](./07-painter.md) →

**Objective:** change a small scene into screen triangles in the internal scratch. If nothing moves in a frame, the row mask must be empty and there must be no SPI.

**Prerequisites:** tasks 4 and 5.

**Read:** [guide 03](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/03-display-st7789.md) (GRAM hold) · [guide 05](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/05-6axis-filter.md), only for the yaw limit, and only if you connect a camera

## Scene

Write one mesh of creature size and a small number of props in a C table in `firmware/main/`. Stay below the limit of **1024** triangles. This is not an asset pipeline.

Keep the tables in usual const data. On the chip, this data can be in flash. This is permitted: at the start of the frame, copy the triangles that you will draw into the internal scratch from task 1. While the filler from task 4 writes pixels, it reads the scratch. It does not read PSRAM or the const mesh.

## Transform and clip

You can use float or Q16 at the vertices. The filler stays integer.

For each frame, from the interpolated pose:

1. Transform and project. Do the perspective divide one time for each vertex.
2. Clip to the 240×240 screen before the raster. Remove the triangles that are not on the screen. Clip the triangles that cross an edge. A six-vertex result, divided into triangles, is sufficient.
3. Write the screen-space triangles into the scratch, with a color index and a sort key for task 7 (a 16-bit depth of the triangle). For now, the color index is flat, or it is a vertex index that you change to one index.
4. Make the row mask from the Y range of each triangle.

If the movement is less than one pixel, or if the pose is the same as the last presented pose, the mask stays empty. Then task 3 does not use SPI.

A flag on the pose, `full_frame`, sets all 240 rows. Usual frames keep it clear. Task 8 sets it. You can also set it when the camera moves by more than one pixel. This is the same concept.

## Camera

The default camera is fixed. If you connect the tilt of the board, use only pitch and roll. Yaw around gravity drifts. Do not use it as a compass. A moving camera is a full-frame event.

## Tests

- One known view: compare a small number of projected vertices with a manual calculation (a cube corner is sufficient).
- A triangle that crosses `x = 0` is clipped. The filler never gets a negative X.
- Two poses that are the same, one after the other: the second present does not call `draw_bitmap`.
- Move one instance by several pixels: only the old and the new Y spans are set in the mask.
- With `full_frame` set: all 240 bits are set, also when the meshes are small.

## Complete when

The five tests pass on the host. `sim.bat` shows the still hold (the window does not flicker from an unnecessary full clear) and the moved instance.

## For later

The painter sort is task 7. This task stores the sort key, but does not use it yet. No textures.
