# 12 — Clips, springs, hitboxes

← [11 sheets](./11-sheets-and-sprites.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 13 Power](./13-power-and-boot.md) →

**Read:** [architecture 6](../../architecture.md#6-runtime-representation-not-a-skeleton) · [architecture 7](../../architecture.md#7-clips-animation-writes-rest-not-pos) · [architecture 8](../../architecture.md#8-physics-and-hitboxes) · golden rules 5–8

**Code:** `Bodies` SoA, one idle rest, springs, one debug clip (wave on `PART_ARM_R`), sphere hitboxes, poke as a *function* you can call from a fake tap (touch chip is lesson 14). `SfxEvt` ring: push on impulse, never block.

## The contract

```
clip.sample(t) → rest[] → spring → pos[] → hitbox
                              └→ project(pos) + sheet(view, clip, frame)
```

A [clip](../glossary.md#clip) is a short animation: keyframed rest poses (and matching sheets), not a film of the whole pet. Clips write **rest**, not `pos`. Animation and the matching sheet frame are the **same pose**. Springs only lag.

Pixels are not physics. [Hitboxes](../glossary.md#hitbox) are spheres on `pos` (and optional `tip`). Never sprite alpha.

## `Bodies`

Six parts: core, head, arm L/R, leg L/R. Simulate in **body space**. Core: Verlet + gravity `-Y` (mapped into body). Limbs: spring to `rest`, little or no extra gravity (or a wave fights you).

```
force = (rest - pos) * k - vel * d
```

While a clip is active, **raise `k`** on masked parts so the hand actually rises. Blend `k` down on clip end.

Idle: `emotion_rest[emotion][part][3]` Q8. Gaze: `rest_head += offset` after clip sample. Additive.

## Clips

`ClipHdr`: id, part_mask, frame_count, fps (~15), duration, `vox_id` (0 = silent). Packed `int16` rest tracks. Playback: lerp two keys, Q8 → float, write `rest[p]`. Missing mask bits keep last emotion rest.

Exporter samples the armature at frame *f* into rest **and** renders sheets at that pose.

L0 walk-cycle film is in (blob, 4 frames × 4 yaws). This lesson is Nest **L2**: locomotion = core translation + yaw plus idle bob; no 5-part walk film.

Lizard **policy** (when to wave) is TBD. Drive `clip_id` from a debug key / UART for now.

## Hitboxes and poke

```
hitbox[i].c = world(pos[i])
hitbox[i].r = radius[i]
```

This lesson is **S / L2** (Nest or Play): core vs floor/walls/toys; limb vs toys; limb/tip vs poke ray. Off: limb vs walls; limb vs limb. **L / L0** later: limb pairs off; screen-space pick ≥24 px; core **and toys** vs scenery AABBs. Hall eat is L0 squash + crumbs, not a Nest clip — later.

Poke (S): unproject tap through `inv(proj*view)`, ray vs spheres, closest hit. Miss → floor ray → walk/look there. Sim: mouse click can feed UV until CST816 exists; that is a **firmware** debug path, not the sim pretending to be a pet.

Squish: core penetration → uniform scale on core blit, ~100 ms recover. Push squish patch on voice A if closing speed beats threshold.

Flinch / shake: `jerk` / `imu_evt` impulse on core+head `vel` (and toys). Core 0 also pokes yelp on voice B (mixer later). Always simulate six masses, even when you later blit L0.

## `SfxEvt`

[SfxEvt](../glossary.md#sfxevt) is overwrite-oldest [SPSC](../glossary.md#spsc), size 8. Core 1 **never waits**. Fields: `id` (patch), `vel` (closing speed), `tag`. Threshold + ~150–250 ms cooldown **per pair** or a rolling core machine-guns the floor.

On impulse, not contact. Pairs that are off stay silent.

## Sim gap

No dual-core, no real mixer. Still: seqlock-shaped snapshot, `g_sfx[]` ring, Core-1-style loop that does not “call mix().” A stub consumer can log events.

## Checkpoint (sim)

Idle L2 part on springs. Fire debug `clip_id` = wave: rest hand rises, mass lags, sheet frame matches the clip. Sphere debug draw. Fake poke hits a sphere, not a pixel. Collide with a toy/floor logs `SfxEvt` with cooldown. Room still gravity-locked and **does not orbit**.

## When the board arrives

Architecture §15 step 6: six springs + idle rest. Sphere poke (S). Play toys. Touch wiring is lesson 14; you can poke with a debug UART until then. PLUS is a lizard one-shot, not camera recenter.

Do not implement lizard personality.

← [11 sheets](./11-sheets-and-sprites.md) · [next: 13 Power](./13-power-and-boot.md) →
