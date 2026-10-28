# 12 — Clips, the core spring, hitboxes

← [11 meshes](./11-sheets-and-sprites.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 13 Power](./13-power-and-boot.md) →

**Read:** [architecture 6](../../architecture.md#6-runtime-representation) · [architecture 7](../../architecture.md#7-clips-animation-writes-bone-locals) · [architecture 8](../../architecture.md#8-physics-and-hitboxes) · golden rules 5–8

**Code:** one `CoreSpring`, six bone locals, FK, one debug clip (a wave on the arm bone), joint spheres, and a poke function that a simulated tap can call. On an impulse, record one impact. Do not put sound in a queue.

## The contract

```
clip.sample(t) → bone local → FK → pose.bone[]
core.rest      → spring     → pose.core
```

A [clip](../glossary.md#clip) writes a **bone-local** rotation. It is not a film, and it is not a rest magnet for each limb. The **core** is the only spring. The appendages do not integrate.

Pixels are not physics. [Hitboxes](../glossary.md#hitbox) are spheres on the bone joints. Never use sprite alpha. Never use mesh triangles.

## Core spring and bones

There are six bones: core, head, arm L/R, leg L/R. The core spring is in **body space**: Verlet integration and gravity `-Y`.

```
force = (rest - pos) * k - vel * d
```

That equation is for the core only. After the clip sample, do FK, so that each bone has a world 3×4 matrix. An optional one-pole jiggle on a bone local is permitted. That is not a spring, and it does not collide.

The skinning stays at triangle setup (lesson 11). This lesson publishes the bone palette.

Gaze: an additive local rotation on the head bone, after the clip sample. Only in S rooms.

## Clips

`ClipHdr` has: id, bone_mask, frame_count, fps (approximately 15), duration, `vox_id` (0 = no chirp), `full_frame`. The data is a packed `int16` local rotation (and an optional translation). Playback: lerp two keys and write `local_q`. If a bit is not in the mask, that bone keeps its bind local. A `vox_id` that is not zero is the chirp to render when the clip starts.

Do **not** export a mesh for each clip frame. A wave is a rotation of the arm bone. The vertices with two influences bend with it.

The lizard **policy** (when to wave) is TBD. For now, use a debug key to set `clip_id`.

## Hitboxes and poke

```
hitbox[i].c = bone[i].joint
hitbox[i].r = bone[i].radius
```

This lesson is for **S** rooms (Nest or Play):

- On: core against floor, walls and toys; arm joints against toys; joint against the poke ray.
- Off: joints against walls; joint against joint.

**L** rooms, later: joint pairs off; a screen-space pick of a minimum of 24 px. The Hall eat is a core squash and crumbs, not a separate mesh. The rigid bodies use one shared array, with a limit of 24 awake. The Nest contents stay quiet, so that a sleep pose can become stable.

Poke (S): unproject the tap through `inv(proj*view)`. Test the ray against the joint spheres and use the nearest hit. If there is no hit, use a floor ray. On the simulator, a mouse click can supply the UV until the CST816 is available.

Squish: penetration of the core → non-uniform `core.scale`, with recovery in approximately 100 ms. If the closing speed is more than the threshold, render an impact.

Shake: an impulse on the core (and on the awake rigids). It is not on five limb springs, because there are no limb springs. A clip can also set `full_frame` if the camera must shake.

## Sound

An [SfxEvt](../glossary.md#sfxevt) has a class, an ID and `vel`. The sim renders it on that tick: a maximum of one impact and one chirp. Use a threshold and a cooldown of approximately 150–250 ms **for each pair**. On silicon, the event goes into the [play buffer](../glossary.md#play-buffer). In this lesson, only record the event.

## Simulator gap

There are no dual cores and no codec. But use a pose slot, and a present loop that does not render audio. When the mailbox is available, a publisher that stops causes Core 1 to hold the last pose.

## Checkpoint (simulator)

- The pet is idle. Send the debug `clip_id` = wave. The arm bone turns and the mesh bends. The core spring does not have to go up for the hand to move.
- The debug spheres show on the joints.
- A simulated poke hits a sphere, not a pixel.
- A hit on the floor or on a toy records one impact, with cooldown.
- The room does **not** turn.

## When the board arrives

Architecture §15 step 6: core spring and FK. Joint poke (S). A small number of rigid toys. Door cut Nest ↔ Hall. Lesson 14 connects the touch.

Do not implement a lizard personality.

← [11 meshes](./11-sheets-and-sprites.md) · [next: 13 Power](./13-power-and-boot.md) →
