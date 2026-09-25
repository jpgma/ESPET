# 12 — Clips, the core spring, hitboxes

← [11 meshes](./11-sheets-and-sprites.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 13 Power](./13-power-and-boot.md) →

**Read:** [architecture 6](../../architecture.md#6-runtime-representation) · [architecture 7](../../architecture.md#7-clips-animation-writes-bone-locals) · [architecture 8](../../architecture.md#8-physics-and-hitboxes) · golden rules 5–8

**Code:** one `CoreSpring`, six bone locals, FK, one debug clip (wave on the arm bone), joint spheres, poke as a function you can call from a fake tap. `SfxEvt` ring: push on impulse, never block.

## The contract

```
clip.sample(t) → bone local → FK → pose.bone[]
core.rest      → spring     → pose.core
```

A [clip](../glossary.md#clip) writes **bone-local** rotation, not a film and not a rest magnet for each limb. The **core** is the only spring. Appendages do not integrate.

Pixels are not physics. [Hitboxes](../glossary.md#hitbox) are spheres on bone joints. Never sprite alpha. Never mesh triangles.

## Core spring and bones

Six bones: core, head, arm L/R, leg L/R. The core spring lives in **body space**: Verlet + gravity `-Y`.

```
force = (rest - pos) * k - vel * d
```

That equation is the core only. After the clip sample, run FK so each bone has a world 3×4. Optional one-pole jiggle on a bone local. That is not a spring and it does not collide.

Skinning stays at triangle setup (lesson 11). This lesson publishes the bone palette.

Gaze: additive local rotation on the head bone after the clip sample. S rooms.

## Clips

`ClipHdr`: id, bone_mask, frame_count, fps (~15), duration, `vox_id` (0 = silent), `full_frame`. Packed `int16` local rotation (and optional translation). Playback: lerp two keys, write `local_q`. Missing mask bits keep the bind local.

Do **not** export a mesh per clip frame. A wave is the arm bone rotating. Vertices with two influences bend with it.

Lizard **policy** (when to wave) is TBD. Drive `clip_id` from a debug key for now.

## Hitboxes and poke

```
hitbox[i].c = bone[i].joint
hitbox[i].r = bone[i].radius
```

This lesson is **S** (Nest or Play): core vs floor/walls/toys; arm joints vs toys; joint vs poke ray. Off: joints vs walls; joint vs joint. **L** later: joint pairs off; screen-space pick ≥24 px. Hall eat is core squash + crumbs, not a separate mesh. Rigid bodies share one array, cap 24 awake. Nest content stays quiet so a sleep pose can settle.

Poke (S): unproject tap through `inv(proj*view)`, ray vs joint spheres, closest hit. Miss → floor ray. Sim: mouse click can feed UV until CST816 exists.

Squish: core penetration → non-uniform `core.scale`, ~100 ms recover. Push squish patch on voice A if closing speed beats threshold.

Shake: impulse on the core (and awake rigids). Not on five limb springs — those are gone. A clip may also set `full_frame` if the camera should shake.

## `SfxEvt`

[SfxEvt](../glossary.md#sfxevt) is overwrite-oldest, size 8. The sim pushes; it does not call the mixer. Fields: `id`, `vel`, `tag`. Threshold + ~150–250 ms cooldown **per pair**.

## Sim gap

No dual-core, no real mixer. Still: a pose slot, `g_sfx[]` ring, and a present loop that does not “call mix().” When the mailbox exists, a stalled publisher holds the last pose.

## Checkpoint (sim)

Idle pet. Fire debug `clip_id` = wave: the arm bone rotates, the mesh bends, the core spring does not have to rise for the hand to move. Joint debug spheres. Fake poke hits a sphere, not a pixel. A floor or toy hit logs `SfxEvt` with cooldown. The room does **not** orbit.

## When the board arrives

Architecture §15 step 6: core spring + FK. Joint poke (S). A few rigid toys. Door cut Nest ↔ Hall. Touch wiring is lesson 14.

Do not implement lizard personality.

← [11 meshes](./11-sheets-and-sprites.md) · [next: 13 Power](./13-power-and-boot.md) →
