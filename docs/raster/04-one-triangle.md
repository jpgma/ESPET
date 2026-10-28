# 04 — One triangle

← [present](./03-cadenced-present.md) · [index](./README.md) · [next: pose mailbox](./05-pose-mailbox.md) →

**Objective:** write one screen-space triangle into the indexed buffer with a fixed fill rule, in increasing Y. A golden pixel list must agree.

**Prerequisites:** the buffer and the present path from task 3.

**Read:** nothing on the chip. This task is the inner loop.

## Code location

`firmware/main/`. The filler includes `stdint.h` and its own header. It does not include ESP-IDF or SDL. The present path from task 3 uses the row mask that the filler sets.

Add the new `.c` file to `SRCS` in [`firmware/main/CMakeLists.txt`](../../firmware/main/CMakeLists.txt). Also add it to the `espet_firmware` sources in [`board-sim/CMakeLists.txt`](../../board-sim/CMakeLists.txt). The sim list links `win32/platform.c`. The silicon list links `esp32/platform.c`.

## Fill rule

Write this rule at the top of the filler and obey it. A shared edge must cover a pixel only one time.

- The center of integer pixel `(x, y)` is `(x + 0.5, y + 0.5)`. Do the test in 16.16 fixed point or in doubled integer coordinates. Do not use floating point in this file.
- All edges use the same winding. Select clockwise and write it in the file.
- A pixel is inside when all edge functions are more than zero, or are zero on a **top** or **left** edge.
- A top edge is a horizontal edge that has the interior below it.
- A left edge is an edge that goes down (toward increasing Y).
- Do not divide for each pixel. The setup can divide one time for each edge, to step the edge function.

Go from the highest covered row to the lowest covered row. In each row, go across the span in X. Set the indexed pixel, and set the bit of that row in the mask.

Send the output through the bands of task 3. After each complete group of `band_rows`, the present side can expand and start DMA. The filler does not call SPI.

## Golden image

Select a small triangle. You must be able to list its covered pixels manually with the rule above. A right triangle of a few dozen pixels is sufficient. Commit the list (coordinates and the color index). Near it, commit a small host check. The check runs the filler into a 240×240 buffer and compares those pixels. It also checks some pixels immediately outside the triangle, which must stay clear.

Run that check from the sim build or from a small separate executable. A triangle in `sim.bat` is the visual part. The list is the proof.

## Complete when

- The golden list agrees, also on the top and left edges that you defined.
- The filler does not change the pixels outside the triangle.
- The filler writes the rows in increasing Y. Only those rows are set in the mask.
- The filler file has no `float` and no ESP-IDF include.
- The triangle shows in `sim.bat` through the present path of task 3.

## For later

No mesh, no clip, no depth sort, no texture. The vertex color is one index on this one triangle.
