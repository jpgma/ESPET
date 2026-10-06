# 04 — One triangle

← [present](./03-cadenced-present.md) · [index](./README.md) · [next: pose mailbox](./05-pose-mailbox.md) →

**Goal:** one screen-space triangle lands in the indexed buffer under a fixed fill rule, in increasing Y, and a golden pixel list matches.

**Depends on:** task 3’s buffer and present.

**Read:** nothing on the chip. This task is the inner loop.

## Where the code lives

`firmware/main/`. The filler includes `stdint.h` and its own header. It does not include ESP-IDF or SDL. The present path from task 3 consumes the row mask the filler sets.

Add the new `.c` file to `SRCS` in [`firmware/main/CMakeLists.txt`](../../firmware/main/CMakeLists.txt) and to the `espet_firmware` sources in [`board-sim/CMakeLists.txt`](../../board-sim/CMakeLists.txt). The sim list links `win32/platform.c`. The silicon list links `esp32/platform.c`.

## Fill rule

Write this rule at the top of the filler and follow it. Shared edges should cover a pixel once.

- Integer pixel `(x, y)` is the center `(x + 0.5, y + 0.5)`. Do the test in 16.16 fixed point or in doubled integer coordinates. No floating point in this file.
- Edges use a consistent winding (pick clockwise and document it).
- A pixel is inside when every edge function is positive, or zero on a **top** or **left** edge.
- A top edge is a horizontal edge whose interior is below it.
- A left edge is an edge that moves downward (toward increasing Y).
- No divide per pixel. Setup may divide once per edge to step the edge function.

Walk Y from the topmost covered row to the bottommost. Inside a row, walk X across the span. Set the indexed pixel and set that row’s bit in the mask.

Ship through task 3’s bands: after each finished group of `band_rows`, the present side may expand and DMA. The filler does not call SPI itself.

## Golden image

Pick a small triangle whose covered pixels you can list by hand under the rule above. A right triangle of a few dozen pixels is enough. Commit the list (coordinates and the color index) next to a tiny host check that runs the filler into a 240×240 buffer and compares those pixels, plus a few pixels just outside that must stay clear.

Run that check from the sim build or a small extra executable. `sim.bat` showing a triangle is the visual half. The list is the proof.

## Done when

- The golden list matches, including the top and left edges you defined.
- Pixels outside the triangle are untouched.
- Rows are written in increasing Y, and only those rows are set in the mask.
- The filler file has no `float` and no ESP-IDF include.
- The triangle shows up through the task 3 present path in `sim.bat`.

## Leave for later

No mesh, no clip, no depth sort, no texture. Vertex color is a single index on this one triangle.
