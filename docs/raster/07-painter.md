# 07 — Painter order

← [mesh and dirty rows](./06-mesh-clip-dirty.md) · [index](./README.md) · [next: full-frame animation](./08-full-frame-cadence.md) →

**Objective:** draw the default scene from far to near. Write down how many indexed pixels the filler stores.

**Prerequisites:** the scratch and the sort key from task 6.

## Sort

Before the filler runs, sort the scratch from far to near with the 16-bit key from task 6. Far means a larger view-space depth. Select the sign when you project, and write it near the sort.

A triangle that is drawn later overwrites the indexed byte. Shared edges still obey the top-left rule of task 4. Thus, two coplanar neighbors do not leave a crack, and the edge does not depend on the sort order.

Count these values:

- The pixels stored (all writes, also the overwrites).
- The pixels in the dirty row span (the rows that have a bit set, multiplied by 240).
- The ratio of those two values.

Write the three values in `budget.md` for the default scene (creature and props, camera that does not move, one pose that moves an item).

## When a z-buffer is permitted

Keep the painter order. Change only if these two conditions are true:

- You can show a case in this scene that paints incorrectly. This must be a mesh that interpenetrates, not only a sort key with the wrong direction.
- The remaining internal heap from task 2 can hold a 16-bit depth for the dirty rows that are important (`rows × 240 × 2`). A full-screen depth buffer is 115200 bytes, and it was not reserved.

If you change, write the case and the byte cost in `budget.md`. Delete the painter path in the same change. Two occlusion paths will become different over time.

## Complete when

- The default scene draws from far to near. Where a near prop and a far prop overlap on the screen, the near prop covers the far prop.
- `budget.md` has the pixel-write count, the dirty-span count and the ratio.
- The occlusion decision is one sentence in `budget.md`: the painter stays, or a z-buffer with its byte cost.

## For later

No SIMD. Task 8 uses the ratio when a full-frame animation makes the dirty span equal to the full screen.
