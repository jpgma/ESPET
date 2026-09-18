# ESPET — Architecture

**A gravity-locked habitat of cube rooms. The screen is a room-authored 3/4 camera. Small rooms put the pet at ~1/3 of the glass; large rooms shrink it to an L0 speck so furniture and toys can exist. IMU jostles contents — it does not orbit the window. Offline-first. Optional LAN cortex.**

Style: data-oriented C/C++ on ESP-IDF. POD tables, integer IDs, no STL in the hot path, no exceptions, no RTTI, no LVGL, no Arduino. One Core 1 loop. As little abstraction as the hardware forces.

**On the chip there is no skinned skeleton.** Six point-masses, clips of rest positions, an atlas of part sheets. An armature exists in Blender only, as the authoring tool.

---

## 0. Product lock

| Decision | Value |
| :--- | :--- |
| Hardware | [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54) (touch + battery) |
| SoC | ESP32-S3R8, dual LX7 @ 240 MHz, 512 KB SRAM, **8 MB in-package octal PSRAM**, 16 MB quad NOR |
| Display | 1.54" IPS **240×240**, ST7789, **4-wire SPI**, RGB565, GRAM on-panel, **no TE pin** |
| IMU | QMI8658 (accel + gyro, **no magnetometer**). Sparse play bus: shake / set-down / face-down. **Not a camera.** |
| Touch | CST816 (I2C, one finger) |
| Audio | **v1.** ES8311 + NS4150B. Procedural **2-voice** mixer on Core 0. ES7210 **off** (mic later). Speaker on MX1.25 **TBD at bring-up**. |
| Storage | TF slot present. **Unused in the frame loop.** Audio is not on the card. |
| Display rate | **30 FPS** cap. Physics in the same 33 ms loop. |
| Battery | ~1000 mAh. **Strive for ~8 h** awake, dim — not a hard cap. Wi-Fi is a luxury mode. |
| Look | Low-poly, hard edges, Spore Creatures (NDS) silhouette. Mock-3D: camera-facing planes, 4 yaws, paper-turn squash. |
| Camera | **Room-authored** 3/4 `look_at` (elev ~40°, azimuth = `room.front`). FOV ~55°. **Never** from IMU `q`. IMU parallax **off** in v1. |
| Pet draw | **L2** (S rooms): 5 part sheets, 64×64, 4 yaws. **L0** (L rooms): 1 blob, 32×32, 4 yaws. **Shadow** is a separate floor stamp, both LODs. LOD is a **room field**. Not a runtime triangle pet. No L1 atlas. |
| Pixels | **Indexed-8** atlas and framebuffer. **Per-room** palette 32, color 0 = key. Indices **1–15** pet/toys/FX (stable across rooms); **16–31** scenery. Expand to RGB565 only on SPI scanout. |
| Room draw | Resident **240×240 indexed backdrop** in PSRAM. Floor **shadow** stamp, then Y-sorted occluders (≤12, S and L), pet, toys, then event **FX**. Not 6 live IMU quads. |
| Map | **S and L only.** v1 rooms: Nest S, Play S, Hall L, Yard L. Kitchen = bowl **prop** in Hall. Eat is **L0 munch** at that bowl. Budget 8 rooms in flash. Do not bake night as extra backdrops. |
| Motion | Clips write **rest**. Springs write **pos**. Hitboxes follow **pos**. L0 walk-cycle film is in. L0 eat = blob squash (hop film or 2 extra frames). |
| FX | Event pool **64**, world `-Y`, floor only. Kinds: **dust, crumbs, leaves**. No alpha. One SFX per burst. |
| Brain | Offline-first. **Lizard-brain *when*** is TBD. Spatial **hooks** are locked (Nest clips, Hall/Yard wander, Play toys, door swap, Hall eat squash, shake → dust/leaves). |
| Parts kit | **Not v1.** Slot IDs exist now so a kit is more atlas + `part_ids[6]`, not a new renderer. |

**Golden rules**

1. Core 1 never waits on Core 0, Wi-Fi, or the LLM.
2. The pet is complete with the radio off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. Sleeping the CPU is a feature. Spinning at 240 MHz with a static frame is a bug. **Silence with I2S clocks or the PA up is the same bug.**
5. Sheets are appearance. Springs are state. Pixels are not physics. **LOD is blit, not sim.**
6. Animation and the matching sheet frame are the **same pose**. Springs only lag that pose.
7. The camera is **room-authored** and static per room. Only yaw `view_idx` (0..3) is discrete.
8. Core 1 never plays audio. It may **emit** an `SfxEvt`. Core 0 mixes.

---

## 1. Gravity-locked habitat, authored camera

Rooms are axis-aligned cubes glued to the Earth. The pet stands on the floor, feet toward real gravity. The physical screen is a **3/4 window on the current room**, not a boom you orbit by tilting. The pet does **not** lean with the glass.

```
                    world +Y (up) = opposite of gravity
                          ^
                          |
                    +-----+-----+  ceiling
                   /     pet     /|
                  +-----+-----+  |     camera = room-authored 3/4
                  |           |  +       (front-face, ~40° elev)
                  |   floor   | /
                  +-----+-----+
                    world XZ
```

Pet height = `1.0` world unit.

### Two play modes

| | **Close (S)** | **Habitat (L)** |
| :--- | :--- | :--- |
| Floor | ~2×2 pet-lengths | ~8×8 pet-lengths |
| Camera | frames **pet** | frames **room AABB** |
| Pet on glass | ~80–100 px | ~24–32 px |
| Draw | **L2** — 5 parts, 4 yaws + shadow stamp | **L0** — 1 blob + shadow stamp, 4 yaws |
| Tap | part spheres | screen-space slop ≥24 px (pet / toys / props) |
| For | wave, poke, sleep, blink | wander, doors, IMU bounce, eat at bowl |
| Dirty rect | ~120×140 | ~32×32 pet + movers; FX may fatten |

No M atlas. No follow-cam in L. Door = smash cut (or ~200 ms full-frame slide).

### v1 map

```
Nest S  ←→  Hall L  ←→  Yard L
              ↕
           Play S
```

| Room | Size | LOD | Role |
| :--- | :--- | :--- | :--- |
| **Nest** | S | L2 | Sleep, poke, close-up clips; blanket `cover_body` occluder |
| **Play** | S | L2 | 1–3 dynamic toys; limb vs ball |
| **Hall** | L | L0 | Wander, doors, bowl prop (eat magnet → L0 munch + crumbs) |
| **Yard** | L | L0 | Wander, scenery, **≤3** toy blobs, IMU bounce, leaf FX on shake |

**Caps per resident room**

| Thing | Cap |
| :--- | ---: |
| Scenery AABBs | 24 |
| Props (tap magnets) | 8 |
| Dynamic toys | **3** in Play S and in an L room |
| Doors | 4 (one per wall, may be none) |
| Occluder billboards | 12 (S and L) |
| Event FX | 64 (resident pool) |

Scenery is **in the backdrop**. Props / toys / occluders / shadow / FX are sprites. Furniture does not rigid-body. Toys collide scenery AABBs. FX collide `y=0` only.

One resident room. Extra rooms later are pak data, not engine work. Flash budget **8** backdrops. Night is a palette, not a second bake.

### Habitat photographs

Art north star (engine hooks, not lizard *when*):

1. Hall: L0 walks behind a plant; **shadow stays on the floor**.
2. Smash cut Hall → Nest (door hole colour continuity).
3. Nest: blanket `cover_body` occluder; **head on top**.
4. Hall bowl: **L0 munch** + crumb burst + one patch. No Kitchen. No L2-in-L.
5. Yard shake: leaves fall with real `-Y`.

Bring-up on silicon still starts at Nest backdrop + bounce (§15). Do not skip to these shots.

### 6-axis IMU: sense always, play sparsely

QMI8658 observes gravity (accel) and angular velocity (gyro). That fully determines **pitch and roll**. It cannot observe **yaw around gravity**. No magnetometer.

Core 0 still runs the complementary filter at 100 Hz and publishes `q_device_to_world`, `grav`, `jerk`. Core 1 **does not** build a camera from `q`. Classify on Core 0; fire one-shots.

| Event | Detect | Play | Rate cap |
| :--- | :--- | :--- | :--- |
| Idle | `\|ω\|` and `\|jerk\|` below epsilon | nothing | — |
| Shake | `jerk` above threshold | impulse on core (+ head in S); toys inherit; **dust** at feet; Yard may emit **leaves** | ~200 ms cooldown, ~5 Hz |
| Set-down | spike then still | one startle | edge |
| Face-down | existing | no new Sfx; finish tail; sleep | edge |
| Held tilt | — | **off in v1** | — |

**Tilt does not change world gravity.** Springs always use `-Y`. Bounce is `vel[]`, not a snow-globe. Gaze may pull the head rest toward a look target additively (S rooms).

PLUS short-press and CST816 double-tap are **one-shots** in the snapshot. They are **not** yaw-recenter (there is no IMU camera yaw). Mapping TBD with the lizard brain (call pet, send to Nest). Not volume.

### Authored camera, discrete yaw sheets

Each room stores a baked `view` / `proj` (or enough to rebuild `look_at`). Same FOV in Blender as runtime.

```
view        = room.view                 // 3/4; elev ~40°; azimuth = room.front
proj        = room.proj                 // perspective ~55°, aspect 1.0
view_idx    = yaw_quad(pet_yaw - room.front)   // 0..3, hysteresis
lod         = room.lod                  // L0 or L2
```

`yaw_quad`: nearest of front / right / back / left. **Hysteresis:** do not switch unless the new winner beats the current by a small margin (e.g. 0.02 on the yaw-direction dot, or a few degrees). Stops texture chatter.

Paper-turn (optional, measure): non-uniform X-scale toward a line, swap sheet, expand. Do not interpolate silhouettes on-chip.

**Impostor:** bake cameras sit on the four yaw headings at the room’s elevation. Runtime camera is that same elevation (S frames the pet, L frames the cube). Pivots: `project(world(pos[i]))` with the room VP. Do **not** also rotate the billboard to a second bake camera.

**Idle SPI:** skip if springs settled, no clip, toys settled, **`fx_live==0`**. **No** `|Δq|` / `|Δcam|` camera test — the window does not twitch with the IMU. USB/studio may **full-frame** while `fx_live`; battery still unions dirty AABBs and holds after the burst.

---

## 2. Hardware map

Confirm once against [refs/ESP32-S3-LCD-1.54-Schematic.pdf](refs/ESP32-S3-LCD-1.54-Schematic.pdf), then treat as law.

| Function | GPIO | Notes |
| :--- | :--- | :--- |
| LCD CS / CLK / MOSI | 21 / 38 / 39 | SPI3, write-only |
| LCD DC / RST / BL | 45 / 40 / 46 | 45 and 46 are strapping pins |
| I2C SCL / SDA | 41 / 42 | **One bus:** QMI8658, CST816, ES8311, ES7210 |
| Touch INT / RST | 48 / 47 | IRQ-driven |
| IMU INT | 6 | FIFO watermark |
| PA enable | 7 | NS4150B. **High only while a voice is live** |
| I2S | 8 MCLK / 9 BCLK / 10 WS / 11 DIN / 12 DOUT | TX to ES8311. DIN unused v1 (mic later) |
| BAT_EN | 2 | **Hold high** or the board dies on battery |
| VBAT ADC / charging | 1 / 3 | Housekeeping |
| PWR / PLUS / BOOT | 5 / 4 / 0 | PWR long-press = latch off. PLUS = lizard one-shot (**not** camera recenter) |
| USB D− / D+ | 19 / 20 | Native USB-CDC / JTAG |

**sdkconfig:** `CONFIG_SPIRAM_MODE_OCT=y`, `CONFIG_SPIRAM_SPEED_80M=y`, quad flash 80 MHz (not OPI), `CONFIG_FREERTOS_HZ=1000`, Bluetooth **off**, Wi-Fi started only in cortex mode, USB CDC on boot.

**SPI:** try 80 MHz; fall back to 40 MHz. Full 240×240 RGB565 @ 40 MHz ≈ 23 ms → that is why 30 FPS and **dirty-rect**. Door cuts may pay the full frame once.

---

## 3. Memory

Wi-Fi DMA cannot live in PSRAM. Dual RGB565 frames (2×115 KB) plus radio is a bad bet.

| Region | Use |
| :--- | :--- |
| **Internal DRAM** | Two **8-bit indexed** framebuffers (2×57.6 KB), **current room 32-entry** RGB565 palette (256-slot table still fine), RGB565 scanline bounce for SPI, `Bodies` + `toys[3]` (~0.5 KB), `FxPool` (~2 KB), room runtime (~0.5 KB), seqlock snapshot, `SfxEvt` ring, two synth voices, 256-sample I2S mix bounce, clip playback scratch, cached `SpriteRec`s (parts + shadow + occ), RTOS stacks. Blit inner loop **never** touches PSRAM. Atlas is indexed-8, index contract §0. Mixer **never** touches PSRAM. |
| **Octal PSRAM** | **Current** room backdrop 57.6 KB. Optional next-room prefetch 57.6 KB. Atlas pixels if XIP cache thrash shows up. Not the framebuffer. Not audio. |
| **16 MB flash** | Firmware, clip tracks, `sprite_id` tables, `SpriteRec`s, atlas, room records, **per-room palettes**, backdrops, 4 yaw headings, FX stamps, `SynthPatch[]`. XIP for cold tables. **No PCM in v1.** |
| **RTC SRAM** | Hunger, happy, sleep, last emotion, **`room_id`**, later `part_ids[6]`. |

**Scanout:** indexed back buffer → expand dirty rows to RGB565 bounce → GDMA to ST7789.

**Backdrop restore:** copy the dirty window **PSRAM → DRAM fb**, then blit sprites in DRAM. Do not keep all 8 backdrops in PSRAM.

Fallback: one RGB565 (115 KB), serialize blit then DMA.

**Budget picture**

| Block | Where | Size |
| :--- | :--- | :--- |
| `Bodies` + toys | DRAM | ~0.5 KB |
| `FxPool` | DRAM | ~2 KB |
| Room runtime | DRAM | ~0.5 KB |
| Room palette | DRAM | 64 B (copy on door) |
| Emotion rests | DRAM or flash | ~200 B |
| Clip tracks | flash | KB |
| `sprite_id` + recs | flash | tens of KB |
| L2 / L0 atlas | flash / PSRAM | see §11 |
| Shadow + FX stamps | flash | few KB |
| Resident backdrop | PSRAM | 57.6 KB |
| `SynthPatch[]` | flash → DRAM copy | ~16 B × N (tiny) |
| 4 yaw headings | DRAM | 48 B |

**Pak (indexed-8).** 64×64 = 4 KB. 32×32 = 1 KB. 240×240 backdrop = 57.6 KB.

| Block | Count | Bytes |
| :--- | ---: | ---: |
| L2 idle | 5 parts × 4 yaws | 80 KB |
| L2 blink | head × 4 × 2 | 8 KB |
| L2 wave | core+arm_r × 4 × 8 | 256 KB |
| L0 pet | idle+walk+hop (+ munch or reuse hop squash) | ~28–36 KB |
| Backdrops | 4 ship / 8 cap | 230 / 461 KB |
| Palettes | 4 rooms × 32 × RGB565 | 256 B |
| Occluders | ≤32 types | ~48 KB |
| Shadow stamps | 2 sizes (S/L) | ~1–2 KB |
| FX stamps | 4–8 of 8×8 or 16×16 | ~2–8 KB |
| Props | ≤24 types | ~24 KB |
| Toys | 3×4 yaws | 12 KB |
| **v1 first-art pak** | idle L2+L0 + 4 backdrops + palettes + props | **~360 KB** |
| **v1 full clips pak** | + wave + blink + 8 rooms | **~0.9–1.1 MB** |
| Firmware | IDF | ~1–1.5 MB |
| **Flash used** | | **~2.5–3 MB / 16 MB** |

TF still unused.

---

## 4. Core allocation

### Core 0 — lizard brain, sensors, mixer, optional radio

| Prio | Job | Rate |
| :--- | :--- | :--- |
| IDF (~22) | Wi-Fi / LwIP | cortex mode only |
| 12 | QMI8658 FIFO + complementary filter + **IMU event classify** | 100 Hz |
| 11 | CST816 IRQ → poke UV, double-tap one-shot | event |
| 8 | Needs, wander, gaze, flinch, clip requests, room walk, cortex inject | 20 Hz |
| 7 | Mixer: fill I2S, PA gate, codec I2C start/stop | 12 kHz / 256-sample block |
| 5 | Backlight, VBAT, BAT_EN, PWR, Wi-Fi up/down | 1–10 Hz |

**One I2C owner** on 41/42. ES8311 register writes only on play start/stop/volume — never inside the 100 Hz IMU drain. ES7210 not initialized.

### Core 1 — the body (one pinned task)

```
forever:
    t0 = CCOUNT
    snapshot shared_state
    apply_imu_evt(snap)             // maybe impulse vel[]; never camera
    sample_clip_or_emotion → rest[] // body space
    rest_head += gaze_offset        // additive, optional; S rooms
    step_springs(dt)                // gravity -Y on core only
    collide()                       // room AABBs + toys; may push SfxEvt
    step_fx(dt)                     // world -Y; floor y=0; kinds dust/crumbs/leaves
    view_idx = yaw_quad(pet_yaw - room.front)
    if dirty:                       // springs, clip, toys, fx_live, view_idx — not |Δq|
        restore_bg(dirty_rect)      // PSRAM backdrop → DRAM fb
        blit_shadow()               // floor stamp at projected core on y=0
        blit_occluders_pet_toys()   // L: depth-sort blob+occ+toys; S: see §10
        blit_fx()                   // event specks on top; no alpha
        wait previous DMA
        kick DMA(dirty_rect)
    sleep_until(t0 + 33.3ms)
```

Lock with `CCOUNT` / gpTimer. If springs settled, no clip, toys settled, **`fx_live==0`**, **do not SPI**. GRAM holds. USB/studio may pay full-frame while FX live.

On door: load neighbor backdrop **and palette** into DRAM/PSRAM, spawn pet on the opposite face, retarget `room.view` / clamp, full-frame (or slide) once.

---

## 5. Inter-core state (seqlock, DRAM only)

Two slots + acquire/release. `volatile` is not a barrier on Xtensa SMP.

```c
typedef struct {
    uint32_t seq;

    float    q_x, q_y, q_z, q_w;   // still published; not a camera input
    float    grav_x, grav_y, grav_z;
    float    jerk;

    uint8_t  imu_evt;           // 0 none, 1 shake, 2 setdown, 3 facedown
    uint8_t  plus;              // one-shot; lizard TBD
    uint8_t  double_tap;        // one-shot; lizard TBD
    uint8_t  room_id;
    uint8_t  emotion;
    uint8_t  action;
    uint8_t  mood;
    uint8_t  cortex_live;
    uint8_t  clip_id;           // 0 = none; lizard/cortex request
    uint8_t  clip_restart;      // one-shot

    float    walk_x, walk_z;
    float    gaze_yaw, gaze_pitch;

    uint8_t  poke;
    float    poke_u, poke_v;
} SharedSnap;

DRAM_ATTR SharedSnap g_shared[2];
_Atomic uint32_t     g_shared_idx;
```

Core 1 copies one coherent snapshot per tick and never reads `g_shared` again that frame.

UDP packets are packed little-endian, **not** this struct. Cortex `target_x/z` is **in-room**. `room_id` in the packet is later; v1 inbox stays as today plus on-device `room_id` in RTC / snap.

### Core 1 → Core 0: sound events

`SharedSnap` does not go this way. Collisions live on Core 1. A tiny overwrite-oldest SPSC; Core 1 **never waits**.

```c
enum { SFX_Q = 8 };

typedef struct {
    uint8_t id;      // patch_id; 0 = none
    uint8_t vel;     // Q8 closing speed → gain / f0
    uint8_t tag;     // pair / part; optional
} SfxEvt;

DRAM_ATTR SfxEvt     g_sfx[SFX_Q];
_Atomic uint8_t      g_sfx_w;
_Atomic uint8_t      g_sfx_r;
```

Full → drop oldest. Core 0 also **pokes the mixer directly** (clip start, flinch / shake yelp). Same consumer, two producers. See §9.

---

## 6. Runtime representation (not a skeleton)

**Authoring:** Blender armature, 6 bones (core parent of the five). Key clips there.

**Device:** 6-wide SoA. No bind-pose inverse, no bone stack, no skinning, no blend tree. **Always simulate 6 masses**, including in L0. LOD does not desimulate limbs.

```c
enum { PART_COUNT = 6 };
enum {
    PART_CORE = 0,
    PART_HEAD,
    PART_ARM_L, PART_ARM_R,
    PART_LEG_L, PART_LEG_R
};

typedef struct {
    float pos[PART_COUNT][3];     // simulated; hitboxes live here
    float vel[PART_COUNT][3];
    float rest[PART_COUNT][3];    // this tick, from idle table or clip
    float k[PART_COUNT];
    float d[PART_COUNT];
    float radius[PART_COUNT];
    float tip[PART_COUNT][3];     // optional; hand/foot for toys
} Bodies;
```

Simulate in **body space** (core at origin, +Y up, yaw = 0). Apply one core 3×4 (world translation + yaw) only for projection, core-vs-room collision, and S-room framing.

Idle / emotion:

```c
int16_t emotion_rest[EMOTION_COUNT][PART_COUNT][3];  // Q8, ~200 B
```

Kit later: `uint8_t part_ids[PART_COUNT]` in RTC. Each id selects an atlas slice for that slot. Same 6-wide arrays.

---

## 7. Clips: animation writes rest, not pos

A wave (hand up) is a **moving magnet**. The spring is the metal.

```
clip.sample(t)  →  rest[]  →  spring  →  pos[]  →  hitbox
                                      └───────→  project(pos) + sheet(lod, view, clip, frame)
```

```c
typedef struct {
    uint8_t  id;
    uint8_t  part_mask;     // bit i set → this part has a track
    uint8_t  frame_count;   // 6–10
    uint8_t  fps;           // 15 is enough
    uint16_t duration_ms;
    uint8_t  vox_id;        // 0 = silent; patch id, play on clip **start**
} ClipHdr;

// Packed after header, only masked parts:
// int16_t rest_xyz[frame_count][popcount(mask)][3];
// int16_t tip_xyz [frame_count][popcount(mask)][3];  // optional
```

Wave, 8 frames, core + arm_r, `int16`×3: **96 bytes** of rest. Tips double that.

Playback: `u = t * fps`, lerp two keys, Q8 → float, write `rest[p]`. Missing mask bits keep last emotion rest.

**Contract:** exporter samples the armature at frame *f* into `rest` **and** renders sheets at that same pose. One pose, two encodings. L2 and L0 sheets for the same pose if both LODs need it (walk is L0-only).

**Stiffness:** while a clip is active, raise `k` on masked parts so the hand actually rises. Blend `k` down on clip end. If `k` is too low, you show a raised-arm sheet on a dangling mass.

**Gaze:** `rest_head += gaze_offset` after the clip sample. Additive. Do not replace the clip. S rooms.

**Vox:** if `vox_id != 0`, Core 0 starts that patch on voice B when the clip **starts** (including cortex-injected `clip_id`). Lizard does not fire a second trigger. Emotion rest tables never auto-vox. `vox_id == 0` is silent, same as `clip_id == 0`.

**L0 walk-cycle film is in v1** (4 frames × 4 yaws, one blob). L0 eat is **blob squash** — reuse hop squash or 2 extra frames × 4 yaws. L2 legs may reuse idle sheets (`0xFFFF` fallback). Close-up locomotion is still core translation + yaw plus idle bob unless a clip exists. Close-up teeth / Nest dish are **not** v1.

---

## 8. Physics and hitboxes

```
force = (rest - pos) * k - vel * d
```

Core: Verlet + world gravity `-Y` (mapped into body space). Limbs: springs to `rest`, no extra gravity (or very little, so arms don’t droop out of the clip).

**Hitboxes are spheres on `pos` (and optional `tip`). Never sprite alpha.**

```
hitbox[i].c = world(pos[i])
hitbox[i].r = radius[i]           // scaled by squish on core
```

| Pair | S / L2 | L / L0 | Why |
| :--- | :--- | :--- | :--- |
| Core vs floor / walls | On | On | Locomotion, squish |
| Core vs scenery AABB | On if scenery | On | walk around furniture |
| Core vs toys | On | On | bounce |
| Toy vs scenery AABB | On if scenery | On | ball under a table |
| Limb vs toys | **On** (Play) | **Off** | wave bops a ball |
| Limb / tip vs poke | **On** | **Off** | boop hand vs nose |
| Limb vs walls / scenery | Off | Off | don’t fight a clip |
| Limb vs limb | Off | Off | not worth it |
| FX vs floor `y=0` | On | On | dust / leaves / crumbs land |
| FX vs scenery / pet / toys | Off | Off | not worth it |

Author clips **inside** the S cube. Collision is not an animation editor. Lizard steers around scenery AABBs in L (no navmesh).

**Poke**

- **S / L2:** unproject the tap through `inv(proj*view)`, ray vs spheres, closest hit. Floor ray if miss → walk / look there.
- **L / L0:** **screen-space** pick. Inflate projected AABBs to **≥24 px**. Nearest of pet / toys / props. Miss → floor walk-to.

**Squish:** core penetration → uniform scale on core blit in S (~100 ms recover). L0: squash the blob sheet (also the **eat** photograph). Push a squish patch (voice A) if closing speed beats the threshold.

**Shake / flinch:** `imu_evt` / `jerk` impulse on core (+ head in S) and on resident toys. Spawn **dust** at feet (and **leaves** if the room record has a leaf emitter). Springs recover into current `rest`. Core 0 pokes a yelp on voice B (not a clip). Cooldown or a twitchy IMU machine-guns bounce. One SFX per FX burst, not per particle.

**Eat (Hall):** bowl is a **prop** magnet, not a fifth room and not L2-in-L. When Core 0 requests eat: L0 blob squash + **crumb** burst at the bowl + **one** patch (`SfxEvt` or mixer poke). *When* is lizard TBD.

**Audio from collide:** on impulse, not contact. Closing speed along the normal above a threshold, plus ~150–250 ms cooldown **per pair**, or a rolling core machine-guns the floor. Push `SfxEvt` and return. Floor impulse also spawns **dust**. See §9.

**Door:** core vs door volume → swap `room_id`, load backdrop **and palette**, spawn on opposite face. Not a physics editor.

---

## 9. Audio (procedural, two voices)

**Locked:** v1, on battery. Mixer on Core 0. Two voices. Procedural patches, **no PCM**. ES7210 off. PA gated. Let a live voice **finish** before chip sleep. 8 h is a strive-for, not a gate that kills chirps.

```
Core 1 collide / squish  →  SfxEvt { id, vel, tag }   // voice A
Core 0 clip start (vox_id), shake/jerk  →  mixer poke  // voice B

Core 0 @ 12 kHz mono, block 256:
    voice A: impact  (noise + one bandpass, decay from vel)
    voice B: creature (2-op FM or square+noise, patch envelopes)
    int32 mix → saturate int16 → I2S GDMA → ES8311
PA GPIO 7 high only while a voice is live
```

**12 kHz, not 24.** This speaker and this look do not need XiaoZhi’s 24 kHz duplex. Confirm with a sine at bring-up; drop to 8 kHz only if the amp/coil is ugly. Steal **pins**, not that audio graph. No MP3, no TF, no AEC.

**Two voices:** a wave that bops a toy **layers** grunt + bop. A third event of the same class replaces that voice; it does not stack. Last-event-wins inside a class.

**Procedural, not samples.** Patches are PODs. The hot path never reads flash for audio. Atlas XIP stays the scarce resource.

```c
typedef struct {
    uint8_t  id;
    uint8_t  kind;       // 0 impact, 1 vox
    uint8_t  osc;        // noise / square / fm
    uint8_t  flags;
    uint16_t dur_ms;
    uint16_t f0, f1;     // start/end Hz
    uint8_t  q;          // resonance
    uint8_t  noise;      // mix
    uint8_t  atk, dec;   // Q8 time
    uint8_t  fm_idx;     // 2-op only
    uint8_t  vel_amt;    // SfxEvt.vel bends f0 / gain
} SynthPatch;            // 16 bytes
```

Voice A: impact patches (floor, wall, ball, furniture bump) — different `f0`/`q`, not four code paths. Voice B: effort / yelp / happy / sleepy. Tuning is data. Idle ambient loops are **off** (they fight PA-gating and sleep).

Collision map: `(pair → patch_id)` in a tiny table. Not art.

**Sleep / GRAM-hold:** Core 1 may skip SPI and WFI while a tail plays. Do **not** light-sleep or deep-sleep the chip, and do not drop PA, until both voices decay to zero (or a hard cap ~800 ms). Face-down: **do not start** new events; let the current tail end; PA low; then sleep.

**Codec:** lazy-init ES8311 on first sound; **standby** between phrases (re-init is tens of ms — a wall hit would miss). Full off only on deep sleep. Volume **fixed** in v1 (studio may be louder, like backlight). PLUS is not volume.

**Speaker:** MX1.25 header. Confirm when the board arrives. Bring-up is I2C ACK + PA pulse + sine; open header is not a missing driver.

**Mic / ES7210:** later. Not initialized. DIN pin unused.

---

## 10. Rendering

### Room

One authored VP per room. Draw order:

```
restore backdrop          // dirty window, PSRAM → DRAM fb
blit shadow               // floor stamp at project(core on y=0); both LODs
blit occluders + pet + toys
blit FX                   // dust / crumbs / leaves; no alpha; on top
```

Do not raster six live IMU quads. Scenery is pixels in the 240×240 indexed backdrop. The blob sheet does **not** contain the puddle — that is how an L0 speck walks behind a plant and the shadow stays on the boards.

**Occluders** are the same list in S and L (cap 12). Sort key is view-space z (still called Y-sort).

- **L:** depth-sort L0 blob + occluders + toys.
- **S:** baked part order per yaw stays. Occluders vs **core**: behind the whole pet, or `cover_body` (Nest blanket) = after non-head parts. **`PART_HEAD` always last** among pet sheets.

**Palette:** copy the room’s 32 RGB565 entries on the door cut (with the backdrop). Index 0 = key. Scanout uses that one table.

### Pet

True billboard: the sprite quad faces the room camera. One `view_idx` (yaw 0..3) for all parts.

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;   // pivot = attach (shoulder, hip, neck); L0 blob = feet
} SpriteRec;

uint16_t sprite_l2[PART_COUNT][4][CLIP_COUNT][MAX_FRAMES]; // 0xFFFF = missing
uint16_t sprite_l0[4][CLIP_COUNT][MAX_FRAMES];
```

Missing entry → idle clip, frame 0, same part and view.

Blit: pivot at `project(world(pos[i]))`. The posed silhouette is **in the pixels**, extending from that hotspot. Do not take an idle-arm stamp and slide it to the hand.

L0: one blob at the projected core, **separate** drop-shadow stamp on the floor, optional squash on bounce or eat. Do not downscale L2 64×64 sheets live. Do not bake the shadow into the blob.

Eyes/mouth: extra small sheets parented to `PART_HEAD`, or baked into the head sheet for v1 (simpler; blink = 2 head frames). L0 skips faces.

Toys: one sheet (4 yaws or 1 view) or a disk. Play S may use a colored sphere raster. L rooms use the same 3-toy cap as Play (L0 disks; no limb collision).

### FX

Not a GPU. DRAM SoA. World space. Gravity `-Y`. Die on `life`. No particle–particle, no scenery AABB, no pet hitboxes.

```c
enum { FX_N = 64 };

typedef struct {
    float   x[FX_N], y[FX_N], z[FX_N];
    float   vx[FX_N], vy[FX_N], vz[FX_N];
    uint8_t life[FX_N];   // ticks left
    uint8_t kind[FX_N];   // 0 dust, 1 crumbs, 2 leaves
    uint8_t pal[FX_N];    // index in 1–15, or stamp id
} FxPool;
```

Stamps: 4–8 types, 8×8 or 16×16, indices in 1–15. Fade = dimmer index or smaller stamp, then free.

Spawn (engine, not personality):

| Hook | Kind |
| :--- | :--- |
| Shake | dust at feet; **leaves** if the room record has an emitter (Yard) |
| Floor impulse | dust |
| Eat at bowl | crumbs |

One `SfxEvt` / mixer poke per burst. Project with the room VP. Draw after toys.

`fx_live` = any `life[i] != 0`. GRAM-hold requires it false.

**Dirty rect:** union of projected moving AABBs + shadow + margin. ST7789 `CASET`/`RASET`. S: 120×140 RGB565 ≈ 7 ms @ 40 MHz. L: 32×48 ≈ 1–2 ms idle; FX scatter may fatten. USB/studio may **full-frame while `fx_live`**. Door cut: full 240×240 ≈ 23 ms once. Battery: still dirty-union and hold after the burst.

**Budget (30 Hz, 40 MHz SPI)**

| Slice | S / L2 | L / L0 | Door cut |
| :--- | ---: | ---: | ---: |
| Clip + springs + toys + AABBs + FX | < 0.6 ms | < 1.0 ms | same |
| Restore dirty from backdrop | ~0.2 ms | trivial | full blit once |
| Sprite blits | 5 parts + shadow 0.5–2 ms | pet+shadow+toys+occ+FX 0.3–1.5 ms | — |
| SPI DMA | **7–12 ms** | **1–3 ms** (full if USB+FX) | **~23 ms** |
| Slack → WFI | rest | rest | drop 1 frame |

No runtime scale. Room frame is authored so one sprite size matches that LOD.

---

## 11. Asset pipeline (PC)

Blender: low poly, hard edges, 16–32 colors, 6-bone armature. Origin of each part mesh at the attach.

**One bake elevation** (~40°, same as runtime rooms). **Four yaw headings** (front, right, back, left) relative to `room.front`. Store the 4 unit XZ headings in the blob; runtime and baker share the table.

For each clip frame *f* that this LOD needs, for each yaw *y*:

1. Set armature to frame *f*.
2. Place camera at the bake elevation / yaw, look at character origin, **same FOV as runtime**. S-scale bake for L2 (pet ~1/3 frame). L-scale bake for L0 (pet ~24–32 px).
3. L2: for each part, hide others, render, crop to alpha bounds, record `hot_x/y` = attach. L0: one blob of the whole pet, hotspot = feet. **Shadow** is a separate floor stamp (S size and L size), not in the blob.
4. Sample bone local translation → `rest` (and tip) `int16` (once per frame, not per yaw).

**Same bake camera for every part of a (y, f) at L2.** Do not look-at each limb centroid.

**Pixel format:** indexed-8. Color 0 = key. **Per-room** `palette[32]` RGB565, copied to DRAM on door. Shared atlas uses a stable index contract: **1–15** pet / toys / FX; **16–31** scenery (Hall wood vs Yard grass). Do not bake a second 240×240 for night.

**Size discipline**

| Content | Indexed | Fits? |
| :--- | :--- | :--- |
| L2 idle 5×4 × 64×64 | 80 KB | Trivial |
| L0 idle+walk+hop | ~28 KB | Trivial |
| L0 munch (or hop squash reuse) | ~0–8 KB | Yes |
| L2 wave 8 frames on **all** parts × 4 yaws | ~2.5 MB extra | Wasteful |
| L2 wave 8 frames on **core + arm_r only** | 256 KB extra | Yes |
| 4 backdrops 240×240 | 230 KB | Yes |
| + 6 emotion poses × 5 parts × 4 yaws | ~1.9 MB extra | Prefer rest-only + `0xFFFF` sheets |

**Clip list and what plays when: TBD** (lizard brain). The pak format allows sparse clips; first art = L2 idle × 4 yaws + L0 idle/walk + 4 backdrops + palettes. Extra emotions/wave/munch/FX stamps are data, not engine work. Clips that should grunt set `vox_id`; idle/emotion tables do not. Hall eat is a Core 0 munch request (squash + crumbs), not a Nest L2 clip.

Rooms in the pak: id, size (S/L), lod, front, view/proj (or look_at params), backdrop off, **`palette[32]`**, scenery AABBs, props, doors, occluders (`cover_body` flag), toy slots, optional leaf **emitter** (pos + count).

Exporter emits one `.pak`: yaw headings, rooms (incl. palettes), clips (`ClipHdr` includes `vox_id`), `sprite_l2` / `sprite_l0`, shadow recs, FX stamps, recs, atlas, backdrops, then audio:

```
patch_count, SynthPatch[patch_count]   // ~16 × N; copy to DRAM at boot
pcm_count = 0                          // reserved; v1 is procedural
pcm_index[]                            // empty
```

Custom packer. Runtime never sees a wav. If patches fail later:

```c
typedef struct {
    uint32_t off;
    uint16_t n_samples;
    uint8_t  rate_div;   // 1 = 12 kHz, 2 = 6 kHz
    uint8_t  fmt;        // 0 = s8, 1 = ima4
} PcmRec;
```

Stream 256 frames into the existing voice. Do not decode MP3. Do not use the TF slot.

**Exporter must fail** if runtime yaw headings and bake cameras disagree by > 1e-5.

---

## 12. Lizard brain — **behaviour TBD**

The toy must run with the radio off. **What it does** (when it wanders, waves, sleeps, thinks) is not locked yet. Do not invent a personality in the engine.

**Machinery that is locked** (so behaviour can be data later):

- `emotion_rest[]` and clip tracks can drive `rest[]`
- Core 0 may set `clip_id` / `emotion` / `walk_x/z` / `room_id` in the snapshot; Core 1 only samples, blends, and collides
- Hunger/happy **storage** in RTC exists; decay rates TBD
- Nest S: close-up clips and poke spheres
- Play S: 1–3 toys; limb vs toy; toy vs scenery AABB if a chair exists
- Hall / Yard L: pick a free 8×8 cell, steer around scenery AABBs; **≤3** toy blobs; toy vs scenery AABB
- Bowl in Hall is an eat **prop** magnet, not a fifth room. Core 0 may request **L0 munch** (squash + crumb FX + one patch). No L2-in-L. Nest dish is not v1
- Door volume → swap room (backdrop + palette)
- Shake is a physics impulse — keep that hook even if the *when* is TBD. Core 0 also pokes voice B (yelp). Core 1 spawns **dust** (and **leaves** if the room has an emitter)
- Floor impulse may spawn dust
- Gaze offset on `rest_head` is an optional hook (S), not a requirement
- `ClipHdr.vox_id` plays on clip start; lizard does not own a parallel vox trigger. `vox_id == 0` is silent
- `collide()` may push `SfxEvt`; threshold/cooldown are engine, not personality

No locked mapping of poke → wave, PLUS → clip, or cortex `action` → pose. Cortex `clip_id` / `emotion` fields stay in the packet as an inbox; ignore `clip_id == 0`. A cortex `clip_id` with a non-zero `vox_id` on that clip still grunts — that is clip data, not a new cortex field. `target_x/z` is in the current room.

---

## 13. Optional cortex (Wi-Fi is a mode)

Radio **off** on battery by default. STA when USB-powered or a persisted “brain” flag and a known AP. No reconnect spin. Failure → lizard.

No JSON, no HTTP in the frame path. UDP, little-endian, LAN, port 8888.

### ESP32 → server (16 bytes)

```c
typedef struct __attribute__((packed)) {
    uint8_t  version;
    uint16_t seq_num;
    int16_t  hunger;
    int16_t  happy;
    int16_t  pos_x, pos_z;
    int16_t  yaw;          // 0..3600
    uint8_t  event;        // 0 heartbeat, 1 poke, 2 shake, 3 wake
    uint16_t crc;
} esp_packet_t;
```

### Server → ESP32 (18 bytes)

```c
typedef struct __attribute__((packed)) {
    uint8_t  version;
    uint16_t seq_num;
    int16_t  target_x, target_z;  // in-room
    uint8_t  action;       // 0 none, 1 play, 2 eat, 3 sleep
    uint8_t  emotion;
    uint8_t  mood;
    uint16_t clip_id;      // maps to on-device clips; 0 = ignore
    uint8_t  reserved[4];  // later: room_id
    uint16_t crc;
} server_packet_t;
```

Heartbeat 5–10 Hz while associated. Immediate ACK. LLM async; inject later. Cache by **event id**, not wrapping `seq_num`.

If/when a think pose exists: set it **immediately** on a cortex-worthy event so the LLM delay is masked. That trigger list is TBD with the lizard brain.

`.NET 10` + Ollama/Llamafile on the PC. The ESP32 does not know.

---

## 14. Power (~8 h, strive)

1000 mAh / 8 h ≈ **125 mA average.** Strive for it. Chirps on battery are in. Idle I2S is not.

Habitat idle is the easy case: tiny dirty rect, IMU at event rate, GRAM-hold when springs/toys settle and **`fx_live==0`**. Close-up play matches old pet-sized SPI. Door cuts are rare full frames. USB may full-frame while FX live; that is studio, not the 8 h path.

| Lever | Default |
| :--- | :--- |
| Backlight PWM | ~30–40% battery; higher on USB; bump on poke, decay |
| ST7789 | Dirty only |
| CPU | 240 MHz interacting; 80/160 after a few seconds still |
| Slack | Core 1 WFI per frame. Chip light-sleep **only if mixer idle** |
| Wi-Fi | Off unless cortex |
| Audio | PA + I2S clocks only while a voice is live. ES7210 off. SD off |
| Deep sleep | Face-down, PWR, long idle — **finish the tail**, PA low, then RTC restore |

USB in = **studio mode** (bright, 30 FPS, no light-sleep, cortex allowed, audio may be louder). Unplug = battery personality. Always hold `BAT_EN`. PWR long-press = latch off.

---

## 15. Development

ESP-IDF ≥ 5.5. C++ as a better C: `-fno-exceptions -fno-rtti`.

`esp_lcd` ST7789 + GDMA. Own `CASET`/`RASET`. IMU = I2C FIFO + filter. CST816 on INT. I2S TX to ES8311. Steal Waveshare **pins only**, not LVGL/XiaoZhi (not the 24 kHz duplex/AEC stack).

**Bring-up order**

1. `BAT_EN`, USB-CDC, octal PSRAM 80 MHz.
2. Backlight + full-screen fill. Print SPI microseconds (40 vs 80).
3. Indexed FB + dirty-rect dummy sprite.
4. Complementary filter → gravity / face-down / `jerk`. **Authored Nest backdrop**, Earth-up. **Product test:** tilt does **not** orbit; shake hops the debug mass.
5. One L2 part, 4 idle yaw sheets, blit at `project(pos)` while `view_idx` follows **pet yaw**. Room stays put.
6. Six springs + one idle rest. Sphere poke (S). Play toys. L0 blob in a dummy Hall. Door cut Nest ↔ Hall.
7. Battery idle / GRAM-hold (springs/toys settled, `fx_live==0`). Studio vs battery backlight.
8. Audio: ES8311 I2C, PA pulse, 12 kHz sine, PA low. Speaker TBD (MX1.25). Then one impact patch from a debug poke; `collide()` → `SfxEvt` (floor, then toy); one clip with `vox_id` plus a toy hit (two voices). GRAM-hold while a tail finishes; confirm PA drops; face-down waits for mixer idle.
9. UDP cortex last (inbox only). Lizard-brain policy later.

---

## 16. Checklist

- [ ] Pins vs [schematic](refs/ESP32-S3-LCD-1.54-Schematic.pdf); SPI mode 0 vs 3
- [ ] `BAT_EN`; PWR latch; VBAT
- [ ] Octal PSRAM 80 MHz; quad flash 80 MHz
- [ ] Cube world-up; pet on `Y=0`; no lean with the glass
- [ ] Camera is room-authored; tilt does not orbit
- [ ] Shake hops; cooldown; dust FX; GRAM-hold when still (`fx_live==0`)
- [ ] Bake FOV / elevation / 4 yaw headings shared; S vs L scale
- [ ] `view_idx` 0..3 hysteresis; L2 5 parts / L0 blob by `room.lod`
- [ ] Indexed atlas + FB; color 0 = key; **per-room palette** (1–15 actors / 16–31 scenery); load on door
- [ ] Resident backdrop in PSRAM; dirty restore; door full-frame once
- [ ] Shadow is a floor stamp, both LODs; not baked into the L0 blob
- [ ] Occluders ≤12 in S and L; L depth-sort with blob/toys; S `cover_body` + head last
- [ ] When a clip exists: posed sheet + `rest` track + spring lag; no double transform
- [ ] S poke hits spheres; L pick is screen-space ≥24 px
- [ ] Dirty-rect 30 FPS; GRAM holds when idle; USB may full-frame while FX live
- [ ] Hall eat = L0 squash + crumbs + one patch; no Kitchen; no L2-in-L
- [ ] `FxPool` 64; dust/crumbs/leaves; floor only; one SFX per burst
- [ ] Toy vs scenery AABB; ≤3 toys in L
- [ ] ES8311 I2C; PA gated; 12 kHz sine; ES7210 off
- [ ] `SfxEvt` from collide (threshold + cooldown); two voices (impact + clip `vox_id`)
- [ ] Tail finishes; PA low; then light-sleep / face-down
- [ ] Wi-Fi off: toy is whole
- [ ] ~8 h dim, radio off (measure; strive)

---

## 17. Suggestions

1. If yaw-sheet pop is harsh, hysteresis first, then optional 1-frame blend of two nearest yaws (2× blit — measure). Never IMU-orbit the camera to “fix” it.
2. **GRAM-hold** is springs/toys settled and `fx_live==0`, not `|Δq|`. Plot bounce events in studio so shake cannot 30 Hz forever.
3. **Bake a debug overlay:** room id, lod, view index, clip, frame, `CCOUNT` of blit. Studio mode only.
4. **Eyes in the head sheet** until blink is worth a second atlas. L0 has no face.
5. **Tip spheres only on arms** when Play toys exist. Until then one sphere per part in S.
6. **Palette 32**, color 0 = key. Per-room table; 1–15 actors / 16–31 scenery. Night, if ever: a second 32-entry table or a dim of 16–31 — **do not** spend extra 240×240 backdrops (keep the 8-cap for rooms).
7. **Exporter must fail** if runtime yaw headings and bake cameras disagree by > 1e-5.
8. **USB = studio**, unplug = battery. No settings menu in v1.
9. **Compile-time SSID** for your LAN.
10. When the kit lands: swap atlas slices, keep clips that only touch attach points (wave still works if the new arm’s hotspot is the shoulder).
11. **Gate PA and I2S** even though 8 h is not a hard cap. Silence with MCLK running is the audio version of a static 30 Hz SPI.
12. If procedural thuds disappoint: fill the pak `pcm` appendix (s8 or ima4 @ 12 kHz) into the **same** voice. Do not add MP3 or the TF slot.
13. Optional later: ≤3° IMU parallax on wall layers. **v1 = 0°.**
14. When a leaf or crumb **settles**, stamp it into the PSRAM backdrop and free the FX slot. Litter dies on door reload. Not v1 (blit inner loop still never touches PSRAM).
15. **2–4 ambient occluder springs** (Hall plant, Yard flower). USB may never settle; battery freezes them so GRAM-hold still works.
16. Nest dish / L2 meal: later. Hall L0 munch is the v1 eat photograph.
17. Window-scissor clouds: skip.

---

## 18. Open questions

**Locked this pass:** room-authored camera. S/L only. Nest / Play / Hall / Yard. L2 vs L0 as a room field. IMU = sparse bounce. Indexed-8. **Per-room palettes** (1–15 / 16–31). Floor **shadow** stamp. Occluders in S and L (≤12; `cover_body` + head last). L toys **3**; toy vs scenery AABB. Hall eat = **L0 munch** + crumbs. Event **FX** 64 (dust/crumbs/leaves). Audio = Core 0 2-voice procedural mixer, `vox_id` on clips, `SfxEvt` from collide, ES7210 off. Lizard-brain *when* = later; spatial hooks exist.

1. **PLUS / double-tap** — call pet, send to Nest, ignore? TBD with the brain. Not volume. Not camera recenter.
2. **Play vs Nest** — two S rooms is locked for v1; could collapse to one S with toys later. Do not invent a fifth room.
3. **Orthonormalize `up`** — leftover for gravity / face-down debug overlays only, not the window. Compile-time switch if you draw a debug horizon.
4. **Part layer order (S).** Baked `draw_order[4][PART_COUNT]` per yaw is **locked** for parts. Occluders vs core, or `cover_body` before head. Do not pos-sort all six parts.
5. **Lizard brain** — whole policy TBD. Hunger decay, wander, gaze, sleep, think, wave, poke mappings, cortex `action` → pose / room. Eat *when* (bowl magnet is locked).
6. **Speaker** on MX1.25 — confirm when the board arrives. Open header is not a driver bug.
7. **Patch tuning** (`f0`, decay, FM index). Machinery is locked; the sounds are data.
8. **Face-down last frame** — prefer a Nest sleep pose vs freeze whatever room you were in? TBD with the brain. The *frame* is art.

**Already locked, restated:** pet-yaw `view_idx`, room VP does not spin when the pet yaws. World down is gravity. No Kitchen. No L2-in-L. No follow-cam in L.

---

The body is a 30 Hz gravity-locked habitat on this Waveshare. Small rooms are the close-up pet. Large rooms are the house: a floor shadow, plants you walk behind, a bowl you munch at, leaves that fall when you shake. IMU shakes the contents. The creature is four yaw photos (five parts or one blob) hung on six springs that chase authored rests. Impacts thud and clips can chirp; the mixer lives on Core 0. The lizard brain does not need a PC. The cortex is a guest.
