# ESPET — Architecture

**A gravity-locked habitat of cube rooms. The screen is a room-authored 3/4 camera. The whole room is live flat triangles, including when the camera is still. The pet is one smooth-skinned mesh on six bones. IMU jostles contents — it does not orbit the window. Offline-first. Optional LAN cortex.**

Style: data-oriented C/C++ on ESP-IDF. POD tables, integer IDs, no STL in the hot path, no exceptions, no RTTI, no LVGL, no Arduino. Core 0 simulates. Core 1 only presents. As little abstraction as the hardware forces.

**On the chip there is a six-bone smooth skin.** Two influences per vertex. Clips write bone-local transforms. The core is still one spring (gravity, squish, shake, floor). Appendages are not springs. An armature in Blender is the authoring tool and the runtime skeleton.

Look references (nearest-neighbor panel pixels, not asset bakes): [refs/look/](refs/look/).

---

## 0. Product lock

| Decision | Value |
| :--- | :--- |
| Hardware | [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54) (touch + battery). Pins, schematic, datasheets: **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154/HARDWARE.md`](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) |
| SoC | ESP32-S3R8 QFN56 **rev v0.2**, dual LX7. **240 MHz confirmed** (`CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240`; the shorter `CONFIG_ESP_DEFAULT_CPU_FREQ_240` does nothing). 512 KB SRAM, **8 MB in-package octal PSRAM** (AP Memory gen3, 3.3 V, test OK), 16 MB quad NOR |
| Display | 1.54" IPS **240×240 visible**, ST7789, **4-wire SPI**, RGB565, controller GRAM **240×320**, **no TE / no MISO**. **Mode 3 at 80 MHz is clear** on this glass. One full frame **12271 µs**. Window the visible 240×240; `CASET`/`RASET` origin is still unlocked. |
| IMU | **QMI8658A** (`WHO_AM_I` `0x05`, `REVISION_ID` `0x7C`), I2C **0x6B** only, INT GPIO6 idle high. No magnetometer. `CTRL1` resets to `0x20` (no address auto-increment, big-endian); set `ADDR_AI` and clear `BE` before a little-endian burst. `CTRL2` resets to ±2 g — write **±8 g (4096 LSB/g)** before using that scale. Start gyro **±1024 dps**, FIFO watermark ~100 Hz. Face-up rest: gravity on **−Z** (~1.03 g), X/Y under 0.16 g, so **+Z is out of the glass**. Sparse play: shake / set-down / face-down. **Not a camera.** |
| Touch | **CST816D** (`ChipID` `0xB6`, proj `0x27`, fw `0x01`), I2C **0x15**, INT 48 idle high, RST 47, **one finger**. The resources pack says CST816T; S/T/D share the map. `EnDClick` exists (`GestureID` `0x0B`); mapping is lizard TBD. |
| Audio | **v1.** ES8311 **0x18** (ID `0x83`/`0x11`) + NS4150B GPIO7. One **12 kHz** play buffer on Core 0; I2S DMA plays it. Impacts are procedural. Creature chirps are **s8** in flash. ES7210 **0x40 never probe** (it ACKs; ID bytes stay `0xFF` until MCLK). MX1.25 speaker is **fitted, non-polarized, and audible** — a 440 Hz sine at 16 kHz was heard. |
| Storage | TF slot present. **Unused in the frame loop.** Audio is not on the card. |
| Display rate | No locked FPS. Glass ceiling **80 Hz** (**12500 µs**). A slower frame runs at its own work time. A quiet frame skips SPI. One ceiling for still frames and `full_frame` clips. |
| Battery | ~1000 mAh. **8 h is parked.** Dirty rows and an empty SPI mask stay so a still pose ships nothing. A full frame is already ~12.3 ms of wire. Play does not need Wi-Fi; this firmware starts it so the heap is the cortex case. |
| Look | Low-poly, hard edges, flat facets, Spore Creatures (NDS) silhouette. **Live raster of the whole room**, always. Three N·L bands at triangle setup. No photograph, no textures, no alpha, no z-buffer. Panel refs: [forest](refs/look/espet_flat_forest_clearing_240.png), [night](refs/look/espet_flat_night_sleep_240.png), [play](refs/look/espet_flat_physics_play_240.png), [close-up](refs/look/espet_flat_closeup_240.png). |
| Camera | **Room-authored** 3/4 `look_at` (elev ~40°, azimuth = `room.front`). FOV ~55°. **Never** from IMU `q`. A scripted shake or close-up sets `full_frame` and may move the camera for that clip only. |
| Pet draw | **One** smooth-skinned mesh, six bones, **two** influences per vertex. Close vs far is the camera and the room size, not a second mesh. **Shadow** is a floor stamp. |
| Pixels | **Indexed-8**, one framebuffer. **Per-room** palette **256** RGB565. Index **0** = key. **1–63** actor ramps (2–3 consecutive shades per material, stable across rooms). **64–255** room. Expand to RGB565 only into DMA bands. |
| Room draw | Live triangles every presented frame: sky bands, slab, props, then the skinned pet and rigid meshes, then the shadow stamp and FX stamps. A still camera may cache static screen-space triangles only when that cache matches a full redraw. |
| Map | **S and L only.** v1 rooms: Nest S, Play S, Hall L, Yard L. Kitchen = bowl **prop** in Hall. Eat is a core **squash** plus crumbs at that bowl. Budget 8 rooms in flash. Night is a palette plus dimmer faces, not a second scene. |
| Motion | Clips write **bone locals**. FK writes bone world matrices. The core spring writes **core pos**. Hitboxes follow **bone joints**. Skinning happens at triangle setup on Core 1. |
| Physics extras | One rigid array, cap **24 awake**. Sleeping bodies drop out of the visible scratch while the camera is still. Nest content stays quiet so a sleep pose can settle. Hall bowl is floor-constrained (slide + yaw, **no flip**). |
| FX | Stamp pool **256**, world `-Y`, floor or life only. Kinds: **dust, crumbs, leaves**. No alpha. No particle–particle. No triangle cards in v1. One SFX per burst. |
| Brain | Offline-first. **Lizard-brain *when*** is TBD. Spatial **hooks** are locked (Nest clips, Hall/Yard wander, Play toys, door swap, Hall eat squash, shake → dust/leaves). |
| Parts kit | **Not v1.** The skeleton is six bones. A later kit swaps the weighted mesh, not the bone count. |

**Golden rules**

1. Core 1 never waits on Core 0, Wi-Fi, or the LLM. A stalled sim holds the last pose.
2. The pet is complete with the radio off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. A still pose does not ship pixels. Spinning at 240 MHz with a static frame is a bug. **Silence with I2S clocks or the PA up is the same bug.** The 8 h target is later; the empty mask is how a still pose ships nothing. The present ceiling is the glass, **12500 µs**.
5. Meshes are appearance. The core spring is state. Bone clips are appearance. Pixels are not physics.
6. Clips write **bone locals**. The core spring only lags the body. Appendages do not spring.
7. The camera is **room-authored** and static per room until a clip sets `full_frame`. IMU tilt does not orbit.
8. Core 1 never plays audio. The sim on Core 0 renders one-shots into the play buffer. I2S DMA plays it.

The raster core this product calls (filler, clip, pose mailbox) is the task list in [`docs/raster`](docs/raster/README.md). This firmware starts Wi-Fi so the internal heap is the cortex case. The pet is still complete with the radio off.

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
| Draw | the same skinned mesh | the same skinned mesh, smaller in frame |
| Tap | bone-joint spheres | screen-space slop ≥24 px (pet / toys / props) |
| For | wave, poke, sleep, blink | wander, doors, IMU bounce, eat at bowl |
| Dirty rect | ~120×140 | ~32×32 pet + movers; FX may fatten |

No follow-cam in L. Door = smash cut (or ~200 ms full-frame slide). A close-up is a **camera clip**, not a room and not a second mesh.

The inspo sheet is the look, not the map. Yard reads as the forest clearing. Nest sleep reads as the night slab. Play reads as the pet and a ball.

### v1 map

```
Nest S  ←→  Hall L  ←→  Yard L
              ↕
           Play S
```

| Room | Size | Role |
| :--- | :--- | :--- |
| **Nest** | S | Sleep, poke, close-up clips. Content stays quiet so a sleep pose can settle. |
| **Play** | S | Toys; limb joints vs ball |
| **Hall** | L | Wander, doors, bowl prop (eat magnet → core squash + crumbs). Bowl **slides + yaws**, does not flip. |
| **Yard** | L | Wander, scenery, toys, IMU bounce, leaf FX on shake |

**Caps per resident room**

| Thing | Cap |
| :--- | ---: |
| Scenery AABBs | 24 |
| Props (tap magnets; may also be rigid) | 8 |
| Awake rigid bodies | **24** |
| Doors | 4 (one per wall, may be none) |
| Visible triangles | **1024** (room + pet + movers) |
| Event FX stamps | 256 |

Sky is a few horizontal quads. A floor is a handful of large triangles, not a grass texture. Static contact darkening is authored darker faces under props. The moving pet's shadow is a floor stamp.

One resident room. Extra rooms later are pak data, not engine work. Flash budget **8** rooms. Night is a palette and dimmer faces, not a second bake.

### Habitat shots

Art north star (engine hooks, not lizard *when*):

1. Hall: the skinned pet walks behind a plant mesh; **shadow stays on the floor**.
2. Smash cut Hall → Nest (door).
3. Nest: a sleep clip on the bones; the core spring settles.
4. Hall bowl: **core squash** + crumb burst + one impact. No Kitchen. Bowl does not flip.
5. Yard shake: leaves fall with real `-Y`. A prop can tip. A scripted camera shake may set `full_frame` for that burst.

Bring-up on silicon still starts at a live Nest slab + bounce (§15). Do not skip to these shots.

### 6-axis IMU: sense always, play sparsely

QMI8658 observes gravity (accel) and angular velocity (gyro). That fully determines **pitch and roll**. It cannot observe **yaw around gravity**. No magnetometer.

Core 0 runs the complementary filter at 100 Hz. It does **not** build a camera from `q`. Classify on Core 0; fire one-shots into the sim, which is also on Core 0.

| Event | Detect | Play | Rate cap |
| :--- | :--- | :--- | :--- |
| Idle | `\|ω\|` and `\|jerk\|` below epsilon | nothing | — |
| Shake | `jerk` above threshold | impulse on the core spring; awake rigids inherit; **dust** at feet; Yard may emit **leaves**; a clip may also set `full_frame` camera shake | ~200 ms cooldown, ~5 Hz |
| Set-down | spike then still | one startle | edge |
| Face-down | existing | no new sound; wait for TX-done; sleep | edge |
| Held tilt | — | **off in v1** | — |

**Tilt does not change world gravity.** The core spring uses `-Y`. Bounce is `vel`, not a snow-globe.

PLUS short-press and CST816 double-tap are **one-shots**. They are **not** yaw-recenter. Mapping TBD with the lizard brain. Not volume.

### Authored camera, optional full-frame clip

Each room stores a baked `view` / `proj` (or enough to rebuild `look_at`). Same FOV in Blender as runtime.

```
view        = room.view                 // 3/4; elev ~40°; azimuth = room.front
proj        = room.proj                 // perspective ~55°, aspect 1.0
light_dir   = room.light_dir            // world-space unit; Nest window, Yard sun
full_frame  = clip wants a moving camera // shake or close-up; else 0
```

A clip that sets `full_frame` may replace `view` for its duration (damped shake, or a dolly onto the pet). When the clip ends, the room view returns. Any camera move of more than a pixel dirties all 240 rows. IMU pitch and roll do **not** drive this in v1.

**Idle present:** Core 1 skips SPI if the interpolated pose matches the last presented pose (bones, rigids, FX, camera). **No** `|Δq|` camera test. A still camera may keep static triangles in screen-space scratch and refill only the dirty window, but that cache must match a full redraw of those triangles. Rigid-body **sleep** is what lets the mask go empty.

---

## 2. Hardware map

Full GPIO table, schematic, strapping traps: [jpgma/esp32-s3 HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) (private). Confirm once against that schematic, then treat as law.

Thin lock (do not invent pins). Drive **`BAT_EN` high in `app_main` before any blocking work.** Host SPI is mux: this product uses **SPI3** (vendor demos use SPI2 — not a net).

| Function | GPIO | Notes |
| :--- | :--- | :--- |
| LCD SPI3 (CS/CLK/MOSI, DC/RST/BL) | 21/38/39, 45/40/46 | write-only, **no TE / no MISO**; 45 and 46 strap |
| I2C (SCL/SDA) | 41/42 | one bus, one owner. Pads are these same wires |
| IMU INT | 6 | FIFO watermark. Product path; hello poll is testbed-only |
| Touch INT / RST | 48 / 47 | IRQ. Do not poll from the IMU drain |
| PA (NS4150B CTRL) | 7 | gate between phrases |
| I2S MCLK/BCLK/WS/DIN/DOUT | 8/9/10/11/12 | DIN=ES7210 (unused). DOUT=ES8311 |
| BAT_EN | 2 | **hold high** (`rtc_gpio_hold` in deep sleep) or the board dies on battery |
| VBAT ADC / CHG_STAT | 1 / 3 | VBAT = VADC **×3**. `CHG_STAT` **active-low** while charging. GPIO3 is also an S3 strap |
| PWR / PLUS / BOOT | 5 / 4 / 0 | PWR long-press = latch off. PLUS is lizard TBD |
| USB D−/D+ | 19/20 | native CDC / JTAG; no CH340. Serial/JTAG **dies in deep sleep** |

I2C 7-bit: QMI8658 **0x6B**, CST816 **0x15**, ES8311 **0x18**, ES7210 **0x40 never probe**. No dedicated RTC IC — persistence is S3 RTC SRAM. TF SDMMC 4-bit (16/15/17/18/13/14) is unused; **GPIO18 is TF D1, not a spare.** Charge current is ISET **160 kΩ**, not software.

**sdkconfig:** `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y`, `CONFIG_SPIRAM_MODE_OCT=y`, `CONFIG_SPIRAM_SPEED_80M=y`, quad flash 80 MHz (not OPI), `CONFIG_FREERTOS_HZ=1000`, Bluetooth **off**, Wi-Fi **started** (the heap test is the cortex case; play is complete with it off), USB CDC on boot. This unit’s flash JEDEC ID is **0x204018** (XMC). The schematic text says W25Q128JVSIQ. IDF reports the chip as `generic`. QIO 80 MHz already boots.

**SPI:** **mode 3, 80 MHz**, picture clear. Full 240×240 RGB565 measured **12271 µs** (wire ≈ 11.5 ms; 40 MHz would be ≈ 23 ms). Dirty rows stay, because a full frame is already ~12.3 ms of wire and a still pose must not ship pixels. A `full_frame` clip or a door pays the full frame. Controller GRAM is **240×320**; window the visible **240×240** and lock offsets before trusting dirty-rect.

---

## 3. Memory

Wi-Fi DMA cannot live in PSRAM. One indexed frame plus the radio is the plan. A second indexed frame is not reserved. A 16-bit z-buffer (115 KB) is not reserved.

| Region | Use |
| :--- | :--- |
| **Internal DRAM** | One **8-bit indexed** framebuffer (57.6 KB). Palette 256×RGB565 (512 B). Two DMA bands, 8 rows: `2 × 8 × 240 × 2 = 7680` B. Screen-space triangle scratch for 1024 tris (~32 KB). Pose mailbox, three slots (camera + six bone matrices + rigid instances). Core spring, `Rigid[24]`, `FxPool` 256, seqlock, **play buffer** (12 kHz × 300 ms `int16` = 7200 B), RTOS stacks. Raster inner loop **never** touches PSRAM. The play buffer stays in DRAM. Playback does not read PSRAM or flash. No z-buffer. No RGB framebuffer. |
| **Octal PSRAM** | Optional cold copies (next-room mesh prefetch, stamp pixels if XIP thrashes). Not the framebuffer. Not the play buffer. Not the pose. Not a backdrop. |
| **16 MB flash** | Firmware, bone clips, weighted pet mesh, room meshes, `MatRamp[]`, room records, **per-room palettes** + `light_dir`, FX/shadow stamps, `ImpactPatch[]`, creature chirps (**s8**). XIP for cold tables and meshes. Chirp bytes are read once when the sim renders. |
| **RTC SRAM** | Hunger, happy, sleep, last emotion, **`room_id`**. |

**Scanout:** indexed frame → expand dirty rows through the palette into a DMA band → GDMA to ST7789. Two bands so expand of band *k+1* overlaps DMA of band *k*.

**Budget picture**

| Block | Where | Size |
| :--- | :--- | :--- |
| Indexed frame | DRAM | 57.6 KB |
| Palette | DRAM | 512 B |
| DMA bands | DRAM | 7.7 KB |
| Triangle scratch (1024) | DRAM | ~32 KB |
| Pose × 3 | DRAM | ~2–4 KB |
| Core spring + `Rigid[24]` | DRAM | ~2 KB |
| `FxPool` 256 | DRAM | ~8 KB |
| Play buffer | DRAM | 7.2 KB |
| Pet mesh + weights | flash XIP | a few KB |
| Room meshes, 4 rooms | flash XIP | tens of KB |
| Palettes | flash | 4 × 512 B |
| Shadow + FX stamps | flash | a few KB |
| Creature chirps (s8) | flash | a few KB |
| Firmware | flash | ~1–1.5 MB |

There is no 57.6 KB backdrop and no 230 KB of room photographs.

---

## 4. Core allocation

### Core 0 — sensors, sim, optional radio

| Prio | Job | Rate |
| :--- | :--- | :--- |
| IDF (~22) | Wi-Fi / LwIP | cortex mode only |
| 13 | **I2C owner** (only task that talks 41/42) | drain / event / codec posts |
| 12 | complementary filter + **IMU event classify** (after owner drain) | 100 Hz |
| 8 | Needs, wander, gaze, flinch, clip requests, room walk, cortex inject | 20 Hz |
| 6 | **Sim:** core spring, FK, rigid bodies, particles, collide, render one-shots, publish pose | 30 Hz |
| 5 | Backlight, VBAT, BAT_EN, PWR, Wi-Fi up/down | 1–10 Hz |

Playback is DMA, so a long solve does not starve I2S. **One I2C owner** on 41/42 is the **only** caller of I2C. GPIO6 (IMU FIFO) and GPIO48 (CST816) ISRs **post** — no I2C in an ISR. The owner drains QMI8658 FIFO and reads a CST816 burst; it never writes ES8311 inside that drain. The sim never talks I2C; it posts codec start/stop/volume. Do not add ES7210 to the device table. The owner serializes the bus.

### Core 1 — present only (one pinned task)

```
forever:
    t0 = deadline                      // absolute 33333 µs, not delay-after-work
    load latest pose                   // atomic acquire; keep previous pose
    interpolate to this deadline       // nlerp bones and instance rotations; lerp translations
    if pose matches last presented:
        sleep until next deadline      // empty mask, no SPI
    else:
        skin + raster into indexed FB  // room, then pet and movers; N·L at setup
        blit shadow + FX stamps
        expand dirty rows into DMA bands
        kick DMA
    sleep until next deadline
```

If a frame is still running when the next tick arrives, count a missed deadline and finish the frame. Do not start a second frame on top of it.

Lock with `CCOUNT` / gpTimer. Core 1 does not step physics, does not read the IMU, and does not touch I2C.

On door: Core 0 loads the neighbor meshes and palette, publishes a pose with `full_frame`, spawns the pet on the opposite face.

---

## 5. Pose mailbox (DRAM only)

Three slots. The publisher never waits for present. The renderer never waits for the publisher. `volatile` is not a barrier on Xtensa SMP.

```c
typedef struct {
    uint32_t seq;
    uint32_t time_us;

    float    cam_view[16];     // or eye/target/up if you rebuild look_at
    float    bone[6][12];      // world 3×4, ~288 B
    float    core_x, core_y, core_z;
    float    core_scale[3];    // squash

    uint8_t  full_frame;       // 1 = all 240 rows
    uint8_t  n_rigid;
    uint8_t  n_fx;
    uint8_t  room_id;
    // followed by packed rigid instances and FX positions for this slot
} Pose;
```

Core 1 loads the latest published slot and the previous one. It interpolates to the **frame deadline**, not to "whenever the sim last ran." If the newest timestamp is older than one period, draw that pose as-is. Do not extrapolate.

A torn slot is rejected (seqlock, or a sentinel word written first and last).

### Sound

The sim renders in place, at most one impact and one chirp on the tick that spawned them. See [§9](#9-audio). No ring.

UDP packets are packed little-endian, **not** `Pose`. Cortex `target_x/z` is **in-room**.

---

## 6. Runtime representation

**Authoring:** Blender armature, 6 bones (core parent of the five). Key clips there. Export one weighted mesh (two influences max) and the bone-local keys.

**Device:** the core is one spring. The other five bones are FK from the clip, plus an optional one-pole jiggle on the local rotation. No appendage springs. No blend tree. No IK.

```c
enum { BONE_COUNT = 6 };
enum {
    BONE_CORE = 0,
    BONE_HEAD,
    BONE_ARM_L, BONE_ARM_R,
    BONE_LEG_L, BONE_LEG_R
};

typedef struct {
    float pos[3];          // simulated core
    float vel[3];
    float rest[3];         // core rest; body space
    float k, d;
    float radius;
    float scale[3];        // squash
} CoreSpring;

typedef struct {
    float local_q[4];      // after clip sample + optional jiggle
    float local_t[3];
    float world[12];       // FK result, copied into the pose
    float joint[3];        // hitbox centre, world
    float radius;
} Bone;
```

Simulate the core in **body space** (origin, +Y up, yaw = 0), then one core 3×4 (world translation + yaw + squash) parents the skeleton.

Skinning is **not** on Core 0. Core 1, per triangle, blends the two bone matrices onto each of the three vertices, then clips and fills. No deformed vertex buffer.

---

## 7. Clips: animation writes bone locals

A wave is a **bone-local rotation** on the arm. The mesh deforms because vertices share that bone and a neighbor. The core spring is separate: a clip may also move `core.rest`.

```
clip.sample(t) → bone local → FK → pose.bone[]
core.rest      → spring     → pose.core
```

```c
typedef struct {
    uint8_t  id;
    uint8_t  bone_mask;
    uint8_t  frame_count;   // 6–10
    uint8_t  fps;           // 15 is enough
    uint16_t duration_ms;
    uint8_t  vox_id;        // 0 = silent; chirp id, render on clip start
    uint8_t  full_frame;    // 1 = camera may move; all rows dirty
} ClipHdr;

// Packed after header, only masked bones:
// int16 quat xyzw + optional int16 translation, per frame
```

A wave of 8 frames on core + arm is a few hundred bytes, not a posed sheet.

Playback: `u = t * fps`, lerp keys, write `local_q` / `local_t`. Missing mask bits keep the bind local. Then FK.

**Gaze:** additive local rotation on `BONE_HEAD` after the clip sample. S rooms. Do not replace the clip.

**Jiggle:** optional one-pole on a bone local after the sample. This is the shake on a leaf or an ear. It is not a spring and it does not collide.

**Vox:** if `vox_id != 0`, Core 0 renders that chirp when the clip **starts**.

**Blink:** a second set of face weights, or a clip that rotates a lid bone if you add one later. v1 lid is two influences on the eye region of the same mesh, or a clip. Do not smuggle a face stamp. L rooms still have the face; it is just small.

---

## 8. Physics and hitboxes

```
force = (rest - pos) * k - vel * d
```

That equation is the **core only**. Limbs do not integrate.

**Hitboxes are spheres on bone joints. Never sprite alpha. Never mesh triangles.**

```
hitbox[i].c = bone[i].joint
hitbox[i].r = bone[i].radius
```

**Rigid bodies** (toys + props): position, orientation, linear/angular velocity, mass, inertia, collider (sphere or box). Impulse solver on Core 0 at 30 Hz. **Sleep** when `|v|` and `|ω|` stay below epsilon; wake on impulse / shake / poke. Cap **24 awake**. Asleep bodies are not copied into the pose while the camera is still.

Hall **bowl:** floor-constrained — slide on `y=0`, yaw free, **no pitch/roll**.

```c
enum { RIGID_N = 24 };

typedef struct {
    float    x, y, z;
    float    vx, vy, vz;
    float    qx, qy, qz, qw;
    float    wx, wy, wz;
    float    mass, inertia;
    uint8_t  shape;      // 0 sphere, 1 box, 2 bowl-slide
    uint8_t  sleep;
    uint8_t  mesh_id;
} Rigid;
```

| Pair | S | L | Why |
| :--- | :--- | :--- | :--- |
| Core vs floor / walls | On | On | Locomotion, squish |
| Core vs scenery AABB | On | On | walk around furniture |
| Core vs rigids | On | On | bounce |
| Rigid vs scenery AABB | On | On | ball under a table |
| Rigid vs rigid | On | On | sandbox; this is the expensive pair |
| Arm joints vs toys | **On** (Play) | **Off** | a wave bops a ball |
| Joint vs poke | **On** | **Off** | boop |
| Joint vs walls / scenery | Off | Off | don't fight a clip |
| FX vs floor `y=0` | On | On | dust / leaves / crumbs land |
| FX vs scenery / pet / toys | Off | Off | not worth it |

A heap of stacked boxes is the frame that costs milliseconds. Free bodies against the floor are cheap. If the solve misses a publish, Core 1 holds the last pose.

**Poke**

- **S:** unproject the tap through `inv(proj*view)`, ray vs joint spheres, closest hit. Floor ray if miss → walk / look there.
- **L:** **screen-space** pick. Inflate projected AABBs to **≥24 px**. Nearest of pet / toys / props. Miss → floor walk-to.

**Squish:** core penetration → non-uniform `core.scale` (~100 ms recover). Same path for the Hall eat photograph. Render an impact if closing speed beats the threshold.

**Shake:** impulse on the core and on awake rigids. Spawn **dust** (and **leaves** if the room has an emitter). A clip may also set `full_frame`. One SFX per FX burst.

**Eat (Hall):** bowl is a **prop** magnet. When Core 0 requests eat: core squash + **crumb** burst + **one** impact. *When* is lizard TBD.

**Door:** core vs door volume → swap `room_id`, load meshes **and palette** + `light_dir`, spawn on the opposite face, `full_frame` once.

---

## 9. Audio

**Locked:** v1. The sim renders one-shots into one buffer. I2S GDMA plays it. There is no mixer task. ES7210 off. PA gated. Let DMA finish before chip sleep. 8 h is parked; chirps are not.

```
Core 0 collide / squish          → render impact   // noise + one-pole, vel scales decay
Core 0 clip start (vox_id)       → render chirp    // s8 from flash, expand to int16
Core 0 shake / jerk              → render chirp

play[3600] int16                 // 12 kHz, 300 ms, 7200 B, internal DRAM, DMA
TX-done ISR                      → stop I2S clocks
then                             → PA low
```

**PA times (NS4150B typical):** raise GPIO7 **≥35 ms** before the first sample (wake). Cold start **≥120 ms** before the first chirp. Render fits in that wait. Do not re-trigger during **~80 ms** shutdown. `ISD` is 1–10 µA with CTRL=0 — PA **must** drop between phrases.

**12 kHz, not 24.** This speaker and this look do not need XiaoZhi’s 24 kHz duplex. A 440 Hz sine at 16 kHz was already heard on this MX1.25 speaker, so the amp and coil are alive; the product rate stays 12 kHz. Steal **pins**, not that audio graph. No MP3, no TF, no AEC.

**Two classes, one buffer.** Impact is a noise burst and a one-pole envelope. Creature is a baked s8 clip (effort / yelp / happy / sleepy), expanded with a gain. A wave that bops a toy still layers: the second class **adds** into the buffer ahead of the DMA read pointer, with a **32-sample** guard so the write stays in front of samples DMA has already fetched. The buffer is internal DRAM, so the add is an ordinary saturate. Last event of a class overwrites from the play cursor forward, with a **5 ms** crossfade (60 samples). At most one render per class per sim tick. Idle ambient loops are **off**.

```c
enum { PLAY_HZ = 12000, PLAY_MS = 300, PLAY_N = 3600 };

typedef struct {
    uint8_t  cls;        // 0 impact, 1 creature
    uint8_t  id;
    uint8_t  vel;        // Q8 closing speed; chirps may ignore
} SfxEvt;                // rendered in place; not a ring

typedef struct {
    uint8_t  id;
    uint8_t  vel_amt;    // how closing speed scales decay and gain
    uint16_t decay_ms;
    uint16_t f0;         // one-pole Hz
    uint8_t  gain;
    uint8_t  pad;
} ImpactPatch;           // 8 bytes

typedef struct {
    uint8_t  id;
    uint8_t  gain;
    uint16_t n;          // samples at 12 kHz, ≤ PLAY_N
    // int8_t pcm[n] follows in the pak
} ChirpHdr;
```

**Sleep:** Core 1 may skip SPI while DMA is still reading. Do **not** light-sleep or deep-sleep the chip, and do not drop PA, until TX-done and the PA has finished shutdown. The buffer length is the cap (300 ms). Face-down: **do not start** new events; let DMA finish; PA low; then sleep.

**Codec:** lazy-init ES8311 on first sound (I2C **0x18**, owner task). **Standby** between phrases (re-init is tens of ms — a wall hit would miss). Full off only on deep sleep. Volume **fixed** in v1 (studio may be louder, like backlight). PLUS is not volume. The sim posts start/stop/volume; it does not write the codec.

**Speaker:** MX1.25 header, non-polarized. A 440 Hz sine at 16 kHz was already heard, so the amp and coil are alive.

**Mic / ES7210:** I2C **0x40**. Two MEMS + analog AEC tap on MIC3. **Never probe. Never init.** DIN pin unused.

---

## 10. Rendering

### Room

One authored VP per room, unless a clip has set `full_frame`. Draw order, far to near:

```
sky bands + slab + static props     // live triangles
skinned pet + rigid meshes          // skin at triangle setup, then the same filler
shadow stamp                        // floor, at the projected core on y=0
FX stamps                           // dust / crumbs / leaves; no alpha; on top
```

**There is no backdrop photograph.** A still camera may reuse screen-space triangles for static meshes. That cache is an optimization. It must match a redraw. The moment the camera moves, drop it and set every row.

**Palette:** copy the room's 256 RGB565 entries on the door. Index 0 = key. **1–63** = actor ramps. **64–255** = this room. Scanout uses that one table.

**Light:** `room.light_dir` world-space unit. At **triangle setup** (after skinning, not in the pixel loop): `band = quantize(N·L, 3)` → write `mat.ramp[band]`. No textures. No Gouraud. No per-pixel N·L.

**Smooth skin** bends elbows. That is a softer silhouette than rigid parts. It is the chosen look. It does not add a texture or a gradient inside a triangle: the triangle is still one flat index.

### Pet

```c
typedef struct {
    uint16_t vtx_off;    // xyz + mat_id + two bone ids + two weights
    uint16_t idx_off;
    uint16_t n_vtx;
    uint16_t n_tri;
} MeshRec;

typedef struct {
    uint8_t ramp[3];     // shadow, mid, lit — indices in 1–63
} MatRamp;

typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;
} SpriteRec;             // stamps only (shadow + FX)
```

Raster one triangle at a time. For a skinned triangle, blend two bone 3×4s onto each vertex, then clip and fill. The filler writes a palette index. It does not see floats.

**Triangle budget (visible, per frame)**

| Set | Tris |
| :--- | ---: |
| Room shell + props | a few hundred |
| Pet | a few hundred vertices, well under ~200 tris |
| Awake rigids | the rest, up to the cap |
| **Cap** | **1024** |

Fill, not the count, is the wall. A still S window (~120×140, ~2× overdraw) is about 1–2.5 ms. A `full_frame` clip at ~2× over the whole glass is about 4–10 ms and overlaps the 23 ms wire at 40 MHz. ~4× overdraw on a moving camera is the frame that misses the glass ceiling. Measure it. If the work is slower, that frame runs at its work time. Do not add a second cadence.

### FX

Stamps, not cards. DRAM SoA. World space. Gravity `-Y`. Die on `life`. No particle–particle. Floor only.

```c
enum { FX_N = 256 };

typedef struct {
    float   x[FX_N], y[FX_N], z[FX_N];
    float   vx[FX_N], vy[FX_N], vz[FX_N];
    uint8_t life[FX_N];
    uint8_t kind[FX_N];   // 0 dust, 1 crumbs, 2 leaves
    uint8_t pal[FX_N];
} FxPool;
```

Spawn: shake → dust (leaves if the room has an emitter); floor impulse → dust; eat → crumbs. One impact per burst.

`fx_live` = any `life[i] != 0`. An empty mask needs a matching pose: camera still, bones still, rigids sleeping, `fx_live==0`.

**Dirty rows:** union of projected moving AABBs, one bit per row. `full_frame` sets all 240. ST7789 `CASET`/`RASET` per band. Door and `full_frame` clips: full 240×240 ≈ 23 ms at 40 MHz, ≈ 11.5 ms at 80 MHz. Tearing is accepted (no TE pin).

**Budget (glass ceiling, 80 MHz SPI)**

| Slice | Still camera | `full_frame` clip |
| :--- | ---: | ---: |
| Core spring + FK + rigids + FX (Core 0) | 1–4 ms typical; a stacked heap can be more | same; a miss holds the pose |
| Skin + raster (Core 1) | dirty window, ~1–3 ms | ~4–10 ms at ~2× |
| SPI DMA | dirty rows | **~12.3 ms** at 80 MHz, overlapped with the filler |
| Slack | only if the frame finished inside 12500 µs | the wire is the long pole |

`board-sim` never rasterizes a habitat. The rasterizer lives in `firmware/` and shows up as ST7789 GRAM pixels.

---

## 11. Asset pipeline (PC)

Blender: low poly, hard edges, flat faces, 6-bone armature. Vertex material id. Each vertex: two bone indices and two weights. Weights sum to 1. Zero the second weight for a rigid vertex.

**Room:** meshes, not a render. Sky quads, a slab of a few triangles, props as their own meshes. Store `light_dir` and the 256-entry palette. Authored darker faces under static props. Do not paint a 240×240 photograph.

**Pet export:**

1. Armature at bind.
2. One mesh, origin at the core, two influences max.
3. **Shadow** is a separate floor stamp.
4. Clip frames: bone-local rotation (and optional translation) `int16`, once per frame. Do not export a mesh per frame.
5. `full_frame` on the clip header if that clip moves the camera.

**Exporter must fail** if a vertex has more than two influences, if a material ramp uses indices outside 1–63, or if a room's visible set exceeds 1024 triangles.

Rooms in the pak: id, size (S/L), front, view/proj, `light_dir`, `palette[256]`, scenery AABBs, props, doors, mesh ids, toy slots, optional leaf emitter.

```
impact_count, ImpactPatch[impact_count]
chirp_count, ChirpHdr[chirp_count] + s8 samples
```

---

## 12. Lizard brain — **behaviour TBD**

The toy must run with the radio off. **What it does** is not locked yet.

**Machinery that is locked:**

- Bone clips and `core.rest` can drive the pose
- Core 0 owns the sim and may set `clip_id` / `emotion` / `walk_x/z` / `room_id` before it publishes
- Hunger/happy **storage** in RTC exists; decay rates TBD
- Nest S: close-up clips and joint poke. Content stays quiet
- Play S: toys; arm joints vs toys
- Hall / Yard L: steer around scenery AABBs; rigids from the shared cap of 24 awake
- Bowl in Hall is an eat **prop** and a floor-constrained rigid. Core 0 may request squash + crumbs. No Kitchen
- Door volume → swap room (meshes + palette + `light_dir`)
- Shake is a core impulse plus dust (leaves if the room has an emitter). A clip may add `full_frame`
- Gaze offset on the head bone is an optional hook (S)
- `ClipHdr.vox_id` renders a chirp on clip start
- `collide()` may render an impact

No locked mapping of poke → wave, PLUS → clip, or cortex `action` → pose. Cortex `clip_id` / `emotion` stay in the packet. `target_x/z` is in the current room.

---

## 13. Optional cortex (Wi-Fi is a mode)

Radio **off** by default. STA when USB-powered or a persisted "brain" flag and a known AP. No reconnect spin. Failure → lizard.

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

## 14. Power (8 h parked)

1000 mAh / 8 h ≈ **125 mA average.** That target is **not** a lock this pass. The frame period is. Chirps stay. Idle I2S stays off.

Panel LED typical **60 mA @ 3.0 V**. At 100% that is about half of 125 mA, so when 8 h returns, battery PWM **30–40%** is physics. Start from vendor **LEDC 5 kHz / 10-bit**. `CHG_STAT` is status, not a play-mode switch. Charge current is the ISET resistor, not firmware.

A still pose skips SPI because the pose matches, not because of the battery. Measured full frame is **12271 µs** at SPI mode 3, 80 MHz. `full_frame` clips and doors pay that ship on purpose.

| Lever | Now | Later, if 8 h returns |
| :--- | :--- | :--- |
| ST7789 | Dirty rows; empty mask when the pose matches | same |
| CPU | 240 MHz while a frame has work; wait only for the rest of the glass period (12500 µs) | 80/160 after a few seconds still |
| Slack | Core 1 waits for the deadline | light-sleep only if DMA is idle and the PA is down |
| Wi-Fi | This firmware starts it so the heap is the cortex case. Play is complete with it off | same |
| Audio | PA + I2S clocks only while DMA is running. ES7210 off. SD off | same |
| Backlight | On for bring-up. USB may be brighter | **30–40%** on battery (60 mA LED) |
| Deep sleep | Face-down and PWR still need TX-done, PA low, `rtc_gpio_hold` on `BAT_EN` | same |

USB in = **studio mode** (bright, glass ceiling, cortex allowed, USB debug). Unplug does not change the raster. Always hold `BAT_EN`. PWR long-press = latch off. Deep sleep ⇒ **no** USB Serial/JTAG.

**v1 wake map** (confirm edges on silicon):

- **Light-sleep** (DMA idle, PA down): timer + CST816 INT **48** + PLUS **4**. IMU INT **6** only if shake should abort sleep. USB debug is off in this state.
- **Deep sleep / PWR latch-off:** hardware (PWR / USB plug), not a software IMU camera. Face-down *pose* is lizard TBD.

---

## 15. Development

ESP-IDF ≥ 5.5. C++ as a better C: `-fno-exceptions -fno-rtti`.

Pins, schematic, and silicon how-to: **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). The raster track there is the filler and the pose mailbox. `esp_lcd` ST7789 + GDMA. Own `CASET`/`RASET` (visible 240×240 of 240×320 GRAM). IMU = I2C FIFO + GPIO6 (poll is hello/testbed only). CST816 on INT 48. I2S TX to ES8311. Steal Waveshare **pins only**, not LVGL/XiaoZhi (not the 24 kHz duplex/AEC stack).

**Bring-up order**

1. `BAT_EN` **first** in `app_main` (hello must drive GPIO2), USB-CDC, octal PSRAM 80 MHz.
2. Backlight + full-screen fill. Print SPI microseconds (40 vs 80). Prefer 80 if the glass is clean.
3. One indexed FB + two DMA bands + dirty rows. An empty mask does not call SPI.
4. Complementary filter → gravity / face-down / `jerk`. **Live Nest slab** (a few triangles, not a photograph), Earth-up. **Product test:** tilt does **not** orbit; shake hops the debug core.
5. One weighted mesh, six bones, two influences, flat N·L. A debug clip turns a bone. The room stays put.
6. Core spring + FK. Joint poke (S). A few rigid toys. Door cut Nest ↔ Hall, `full_frame` once.
7. Pose mailbox: Core 0 publishes, Core 1 interpolates. A hitching sim does not move the deadline. Empty mask when the pose matches.
8. Audio: ES8311 I2C, PA pulse, 12 kHz sine, PA low. Then a collide impact and one clip with `vox_id` render into the play buffer. TX-done stops the clocks; PA drops; face-down waits for DMA idle.
9. UDP cortex last (inbox only). Lizard-brain policy later.

---

## 16. Checklist

- [x] Pins vs [HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md). SPI **mode 3 at 80 MHz**, image clear, frame 12271 µs
- [ ] `CASET`/`RASET` origin
- [ ] `BAT_EN` first; PWR latch; VBAT ×3; `CHG_STAT` active-low
- [x] Octal PSRAM 80 MHz (AP gen3, test OK); quad flash 80 MHz (JEDEC `0x204018`)
- [ ] Cube world-up; pet on `Y=0`; no lean with the glass
- [ ] Camera is room-authored; tilt does not orbit; `full_frame` is a clip flag
- [ ] One indexed FB; two DMA bands; palette 256; 0 = key; 1–63 actor ramps; 64–255 room
- [ ] No backdrop photograph; sky is quads; floor is a few large triangles
- [ ] Six-bone smooth skin, two influences; skin at triangle setup; core is the only spring
- [ ] Pose mailbox, three slots; Core 1 interpolates to the deadline; a stall holds the last pose
- [ ] Visible triangles ≤ 1024; scratch stays internal
- [ ] Shadow is a floor stamp; N·L at setup; no textures, no z-buffer
- [ ] Awake rigids ≤ 24; bowl does not flip; Nest content stays quiet
- [ ] `FxPool` 256 stamps; dust/crumbs/leaves; floor only; one SFX per burst
- [ ] Empty mask when the pose matches; `full_frame` ships all 240 rows
- [ ] Hall eat = core squash + crumbs + one impact; no Kitchen
- [ ] One I2C owner; QMI8658 0x6B FIFO; CST816 0x15 on INT 48; ES8311 0x18; ES7210 never probed
- [ ] PA ≥35 ms wake / ≥120 ms cold; 12 kHz sine; ES7210 off
- [ ] Collide renders an impact; a clip renders a chirp; TX-done before sleep
- [ ] Light-sleep wake: timer + INT 48 + PLUS 4 (IMU 6 only if shake-to-wake)
- [ ] GPIO18 is not a spare
- [ ] Wi-Fi off: toy is whole
- [ ] 8 h is parked, not a gate
- [ ] Rasterizer is firmware; `board-sim` never rasterizes a habitat

---

## 17. Suggestions

1. Never IMU-orbit the camera. A shake that should move the glass is a `full_frame` clip, not `q`.
2. **Empty mask** is "pose matches," not `|Δq|`. Overlay awake-body count. A jittering prop keeps the mask hot.
3. **Debug overlay:** room id, clip, frame, awake rigids, `full_frame`, `CCOUNT` of raster, VBAT ×3, backlight PWM. Studio only.
4. **Blink** stays on the skinned mesh. Do not add a face stamp.
5. **Palette 256**, with actor ramps reserved in 1–63, so a night cap keeps its red.
6. **Exporter fails** on a third influence, a ramp outside 1–63, or a room over 1024 tris.
7. **USB = studio**, unplug = same raster. No settings menu in v1.
8. **Compile-time SSID** for your LAN.
9. A later kit swaps the weighted mesh and keeps the six bones, so a wave clip still addresses the arm.
10. **Gate PA and I2S.** Silence with MCLK running is the audio version of a still frame that still ships SPI. PA ≥35 ms wake / ≥120 ms cold; do not re-trigger during ~80 ms shutdown.
11. Thuds stay procedural so velocity can scale the decay. Creature lines are already s8 chirps in the same buffer. Do not add MP3 or the TF slot.
12. Optional later: ≤3° IMU parallax. **v1 = 0.** That parallax would be `full_frame` every tick. Do not turn it on casually.
13. **N·L at setup, not in the pixel loop.** Flat bands are the look and the budget. Index-Gouraud stays parked.
14. **Leaves as 2-tri cards** can wait. Stamps are v1.
15. **SPI mode 3 at 80 MHz is clear** on this glass. A full frame is ~12.3 ms. Dirty rows keep a partial update under the glass, and a still pose ships nothing.
16. **Bowl does not flip.**
17. If a `full_frame` heap of rigids misses the glass ceiling, that frame runs at its work time. Do not add a second clock.
18. Sim: **click-without-drag = CST816 tap**; drag past a few px = IMU. Do not overload left-drag.
19. **`DisAutoSleep`** on CST816 from day one; first-tap-eaten is the usual foot-gun.
20. GPIO3 is `CHG_STAT` and an S3 strap — bring-up note if USB-in-at-reset fights Serial/JTAG.
21. **Do not add a debug LED on GPIO18** (TF D1).
22. Lock `CASET`/`RASET` offsets before trusting dirty-rect.

---

## 18. Open questions

**Locked this pass:** live flat room, no photograph. Room-authored camera, `full_frame` only on a clip. Nest / Play / Hall / Yard. One six-bone smooth skin, two influences, core spring only. Pose mailbox: Core 0 simulates, Core 1 presents. Indexed-8, one framebuffer, two DMA bands, palette 256 (1–63 actor / 64–255 room). Awake rigids **24**. FX **256** stamps. 8 h parked. SPI **mode 3 at 80 MHz** (12271 µs). One I2C owner. I2C addrs 0x6B / 0x15 / 0x18. PA gate times. ES7210 **never probed**. No RTC IC. GPIO18 is not a spare. CST816 `EnDClick` **exists** (`ChipID` `0xB6`). Light-sleep wake: timer + 48 + 4. One 12 kHz play buffer (300 ms, DRAM). Impacts procedural. Chirps s8. No mixer task. Lizard-brain *when* = later.

1. **PLUS / double-tap** — call pet, send to Nest, ignore? TBD with the brain. Not volume. Not camera recenter. Hardware can raise `GestureID` `0x0B`; policy is still lizard.
2. **Play vs Nest** — two S rooms is locked for v1. Do not invent a fifth room.
3. **Lizard brain** — whole policy TBD. Hunger decay, wander, gaze, sleep, think, wave, poke mappings, cortex `action` → clip / room. Eat *when* (bowl magnet is locked).
4. **Impact decay and chirp takes.** Machinery is locked; the sounds are data. The MX1.25 coil already answered a 440 Hz sine.
5. **Face-down last frame** — Nest sleep pose vs freeze the current room? TBD with the brain.
6. **How hard a `full_frame` shake may be** — triangle cap and overdraw are locked. The ceiling is the glass (12500 µs). A slower frame runs at its work time. There is no second clock.

**Already locked, restated:** the room is triangles every frame. The pet is one skinned mesh. World down is gravity. No Kitchen. No follow-cam in L. No 4-yaw atlas. No backdrop. No z-buffer. Bowl does not flip. This firmware starts Wi-Fi so the heap is the cortex case. The pet is complete with the radio off. One I2C owner. ES7210 never probed. GPIO18 is not a spare.

---

The body is a gravity-locked habitat on this Waveshare. Rooms are live flat dioramas: a slab, a few props, a sky of hard bands. The creature is one smooth skin on six bones, with a spring only in the core, and a floor stamp under its feet. Toys can knock around, then sleep. A clip may shake the camera or push it close; every other frame keeps the room's window. Impacts thud and clips can chirp. The lizard brain does not need a PC. The cortex is a guest.
