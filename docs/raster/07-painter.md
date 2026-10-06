# 07 — Painter order

← [mesh and dirty rows](./06-mesh-clip-dirty.md) · [index](./README.md) · [next: full-frame animation](./08-full-frame-cadence.md) →

**Goal:** draw the default scene far to near, and write down how many indexed pixels the filler stores.

**Depends on:** task 6’s scratch and sort key.

## Sort

Before the filler runs, sort the scratch far to near by the 16-bit key from task 6. Far means larger view-space depth (pick the sign when you project, and document it next to the sort).

A triangle drawn later overwrites the indexed byte. Shared edges still follow task 4’s top-left rule, so two coplanar neighbors do not leave a crack and do not depend on sort order for the edge.

Count:

- Pixels stored (every write, including overwrites).
- Pixels in the dirty row span (rows with a bit set, times 240).
- The ratio of those two.

Put the three numbers in `budget.md` for the default scene (creature plus props, camera still, one pose that actually moves something).

## When a z-buffer would be allowed

Keep painter order. Switch only if you can show a case in this scene that paints wrong (a mesh that interpenetrates, not merely a sort key that was backwards), **and** task 2’s remaining internal heap can hold a 16-bit depth for the dirty rows you care about (`rows × 240 × 2`). A full-screen depth buffer is 115200 bytes and was not reserved.

If you switch, write the case and the byte cost in `budget.md` and delete the painter path in the same change. Two occlusion paths will drift.

## Done when

- The default scene draws far to near. A near prop covers a far one where they overlap on screen.
- `budget.md` has the pixel-write count, the dirty-span count, and the ratio.
- The occlusion choice is one sentence in `budget.md`: painter stays, or a z-buffer with its byte cost.

## Leave for later

No SIMD. The ratio is the number task 8 uses when a full-frame animation makes the dirty span the whole screen.
