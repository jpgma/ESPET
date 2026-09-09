# ESPET — Architecture

**A gravity-locked cube. The screen is a continuous IMU boom camera. The pet’s *appearance* snaps among 20 dodecahedron-baked sheets. Offline-first. Optional LAN cortex.**

Style: data-oriented C/C++ on ESP-IDF. POD tables, integer IDs, no STL in the hot path, no exceptions, no RTTI, no LVGL, no Arduino. One Core 1 loop. As little abstraction as the hardware forces.

**On the chip there is no skinned skeleton.** Six point-masses, clips of rest positions, an atlas of part sheets. An armature exists in Blender only, as the authoring tool.

---

## 0. Product lock

| Decision | Value |
| :--- | :--- |
| Hardware | [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54) (touch + battery) |
| SoC | ESP32-S3R8, dual LX7 @ 240 MHz, 512 KB SRAM, **8 MB in-package octal PSRAM**, 16 MB quad NOR |
| Display | 1.54" IPS **240×240**, ST7789, **4-wire SPI**, RGB565, GRAM on-panel, **no TE pin** |
| IMU | QMI8658 (accel + gyro, **no magnetometer**) |
| Touch | CST816 (I2C, one finger) |
| Audio | ES8311 + ES7210 + NS4150B present. **Powered down in v1.** |
| Storage | TF slot present. **Unused in the frame loop.** |
| Display rate | **30 FPS** cap. Physics in the same 33 ms loop. |
| Battery | ~1000 mAh. Target **~8 h** awake, dim. Wi-Fi is a luxury mode. |
| Look | Low-poly, hard edges, Spore Creatures (NDS) silhouette. The **room and pivots** orbit smoothly. The **pet pixels** facet every ~37° (20 sheets). |
| Camera | Boom, look-at = **pet core**. **Continuous** IMU. Never quantized to the dodecahedron. Elevation clamped ≥ ~12° (no peek-under). |
| Pet draw | **Part sheets** baked from 20 dodecahedron cameras. Runtime picks the nearest sheet. Not a runtime triangle pet. |
| Pixels | **Indexed-8** atlas and framebuffer. Palette 32, color 0 = key. Expand to RGB565 only on SPI scanout. |
| Room draw | **6 quads** (floor, 4 walls, optional ceiling). Raster from the **live** view/proj. |
| Motion | Clips write **rest**. Springs write **pos**. Hitboxes follow **pos**. |
| Brain | Offline-first. **Lizard-brain behaviour, clip list, and animation triggers are TBD.** The machinery (rest tables, clips, `clip_id` inbox) exists; what *starts* a wave is not locked. |
| Parts kit | **Not v1.** Slot IDs exist now so a kit is more atlas + `part_ids[6]`, not a new renderer. |

**Golden rules**

1. Core 1 never waits on Core 0, Wi-Fi, or the LLM.
2. The pet is complete with the radio off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. Sleeping the CPU is a feature. Spinning at 240 MHz with a static frame is a bug.
5. Sheets are appearance. Springs are state. Pixels are not physics.
6. Animation and the matching sheet frame are the **same pose**. Springs only lag that pose.
7. The camera is continuous. Only `view_idx` (which photo) is discrete.

---

## 1. Gravity-locked cube, continuous camera

The virtual room is a cube glued to the Earth. The pet stands on the floor, feet toward real gravity. The physical screen is a window. Tilt/roll orbits the window **smoothly**; the pet does **not** lean with the glass.

```
                    world +Y (up) = opposite of gravity
                          ^
                          |
                    +-----+-----+  ceiling
                   /     pet     /|
                  +-----+-----+  |     camera = IMU boom (continuous)
                  |           |  +  <---- sheets: nearest of 20 vertices
                  |   floor   | /
                  +-----+-----+
                    world XZ
```

### 6-axis IMU: tilt yes, compass no

QMI8658 observes gravity (accel) and angular velocity (gyro). That fully determines **pitch and roll**. It cannot observe **yaw around gravity**. No magnetometer.

| Motion | Result |
| :--- | :--- |
| Tilt, roll, lay flat (top-down) | Solid. Accel defines down; gyro smooths it. |
| Spin on the table | Gyro-only yaw. **Drifts.** Recenter required. |
| Shake | High-pass / jerk → flinch. **Not** fed into the camera. |

### Continuous camera, discrete sheets

Core 0 publishes a unit quaternion `q_device_to_world` at 100 Hz (complementary filter: gyro integrate, accel pulls down).

Core 1 builds the **actual** camera from that quaternion. The dodecahedron is **only** a texture index.

```
world_up    = (0, 1, 0)
forward     = rotate(q, {0,0,-1})          // into the screen; confirm on bring-up
up          = rotate(q, {0,1, 0})
focus       = pet core position            // locked: not cube center
cam_pos     = focus - forward * boom_length
// hemisphere clamp (locked): lift cam_pos if elevation < ~12°
view        = look_at(cam_pos, focus, up)  // live IMU; not a vertex
                                           // `up`: see open questions — try orthonormalize to +Y first
proj        = perspective(fov, 1.0, near, far)

dir_world   = normalize(cam_pos - focus)
dir_local   = inv_R_core * dir_world        // pet yaw cycles sheets, not the room
view_idx    = argmax(dot(dir_local, dodeca_vertex[i]))   // 20 dots, hysteresis
```

`dodeca_vertex[]` is 20 unit vectors. **Vertex 0 = +Y** so a top-down hold picks a real top-down sheet.

Neighboring vertices are ~**37°** apart (`arccos(√5 / 3)`). The **window** does not jump. The **photos** do, when `view_idx` changes.

**Hysteresis on sheets only:** do not switch `view_idx` unless the new winner beats the current by a small margin (e.g. 0.02 on the dot). Stops texture chatter on a boundary. The camera keeps moving.

**Yaw policy:** PLUS short-press and CST816 double-tap recenter. Optional: decay yaw toward “front” while `|gyro|` is tiny.

**Hemisphere (locked):** clamp the **camera** elevation to ≥ ~12°. No looking up through the floor. Bake all 20 vertices anyway (peek-under can unlock later). `view_idx` = nearest vertex **inside that hemisphere**, so a clamp at 12° does not pick a below-horizon sheet.

**Tilt does not change world gravity.** Springs always use `-Y`. Shake still flinches. Gaze may pull the head rest toward `cam_pos` additively.

**Impostor error (accepted):** bake cameras sit on vertices; the live camera sits between them (up to ~18°). Pivots are projected with the live VP (smooth, correct 3D positions). Pixels are a photo from nearby. Between snaps you get some parallax mismatch between parts. That is the cost of a smooth window. Do **not** snap `cam_pos` to a vertex to “fix” this.

**Idle SPI:** a continuous IMU always twitches. Require `|Δcam|` and `|Δq|` below epsilon (and springs settled, no clip) before skipping a frame. Quantized cameras got this for free; this one will not.

Boom length: FOV ~55°, pet ~1/3 of the frame, camera kept outside the cube. Same FOV/boom in Blender so sheet scale is in the right ballpark; they will not match pixel-perfect off-vertex.

---

## 2. Hardware map

Confirm once against the [schematic](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.54/ESP32-S3-LCD-1.54-Schematic.pdf), then treat as law.

| Function | GPIO | Notes |
| :--- | :--- | :--- |
| LCD CS / CLK / MOSI | 21 / 38 / 39 | SPI3, write-only |
| LCD DC / RST / BL | 45 / 40 / 46 | 45 and 46 are strapping pins |
| I2C SCL / SDA | 41 / 42 | **One bus:** QMI8658, CST816, ES8311, ES7210 |
| Touch INT / RST | 48 / 47 | IRQ-driven |
| IMU INT | 6 | FIFO watermark |
| PA enable | 7 | Keep low |
| I2S | 8 / 9 / 10 / 11–12 | Unused v1 |
| BAT_EN | 2 | **Hold high** or the board dies on battery |
| VBAT ADC / charging | 1 / 3 | Housekeeping |
| PWR / PLUS / BOOT | 5 / 4 / 0 | PWR long-press = latch off. PLUS = recenter |
| USB D− / D+ | 19 / 20 | Native USB-CDC / JTAG |

**sdkconfig:** `CONFIG_SPIRAM_MODE_OCT=y`, `CONFIG_SPIRAM_SPEED_80M=y`, quad flash 80 MHz (not OPI), `CONFIG_FREERTOS_HZ=1000`, Bluetooth **off**, Wi-Fi started only in cortex mode, USB CDC on boot.

**SPI:** try 80 MHz; fall back to 40 MHz. Full 240×240 RGB565 @ 40 MHz ≈ 23 ms → that is why 30 FPS and **dirty-rect**.

---

## 3. Memory

Wi-Fi DMA cannot live in PSRAM. Dual RGB565 frames (2×115 KB) plus radio is a bad bet.

| Region | Use |
| :--- | :--- |
| **Internal DRAM** | Two **8-bit indexed** framebuffers (2×57.6 KB), **32-entry** RGB565 palette (256-slot table still fine), RGB565 scanline bounce for SPI, `Bodies` SoA (~1 KB), seqlock snapshot, clip playback scratch, 5 cached `SpriteRec`, RTOS stacks. Blit inner loop **never** touches PSRAM. Atlas is indexed-8, same palette. |
| **Octal PSRAM** | Atlas pixels if XIP cache thrash shows up. Not the framebuffer. |
| **16 MB flash** | Firmware, clip tracks, `sprite_id` table, `SpriteRec`s, atlas, 20 vertices. XIP for cold tables. |
| **RTC SRAM** | Hunger, happy, sleep, last emotion, later `part_ids[6]`. |

**Scanout:** indexed back buffer → expand dirty rows to RGB565 bounce → GDMA to ST7789.

Fallback: one RGB565 (115 KB), serialize blit then DMA.

**Budget picture**

| Block | Where | Size |
| :--- | :--- | :--- |
| `Bodies` | DRAM | ~1 KB |
| Emotion rests | DRAM or flash | ~200 B |
| Clip tracks | flash | KB |
| `sprite_id` + recs | flash | tens of KB |
| Atlas pixels | flash / PSRAM | **the** budget (MB) |
| 20 vertices | DRAM | 120 B |

---

## 4. Core allocation

### Core 0 — lizard brain, sensors, optional radio

| Prio | Job | Rate |
| :--- | :--- | :--- |
| IDF (~22) | Wi-Fi / LwIP | cortex mode only |
| 12 | QMI8658 FIFO + complementary filter | 100 Hz |
| 11 | CST816 IRQ → poke UV, double-tap recenter | event |
| 8 | Needs, wander, gaze, flinch, clip requests, cortex inject | 20 Hz |
| 5 | Backlight, VBAT, BAT_EN, PWR, Wi-Fi up/down | 1–10 Hz |

**One I2C owner** on 41/42.

### Core 1 — the body (one pinned task)

```
forever:
    t0 = CCOUNT
    snapshot shared_state
    sample_clip_or_emotion → rest[]     // body space
    rest_head += gaze_offset            // additive, optional
    step_springs(dt)                    // gravity -Y on core only
    collide()                           // see §8
    build_live_camera()                 // IMU boom; not a vertex
    view_idx = nearest_dodeca(dir_local) // sheets only
    if dirty:                           // springs, clip, view_idx, or |Δcam| > eps
        restore_bg(dirty_rect)
        raster_room_quads()             // live view/proj
        blit_parts()                    // 5 color-key sprites, that view_idx
        wait previous DMA
        kick DMA(dirty_rect)
    sleep_until(t0 + 33.3ms)
```

Lock with `CCOUNT` / gpTimer. If springs settled, no clip, and `|Δq|` / `|Δcam|` below deadband, **do not SPI**. GRAM holds. Same `view_idx` is not enough — the cube still slides between sheet switches.

---

## 5. Inter-core state (seqlock, DRAM only)

Two slots + acquire/release. `volatile` is not a barrier on Xtensa SMP.

```c
typedef struct {
    uint32_t seq;

    float    q_x, q_y, q_z, q_w;
    float    grav_x, grav_y, grav_z;
    float    jerk;

    uint8_t  recenter;          // one-shot
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

UDP packets are packed little-endian, **not** this struct.

---

## 6. Runtime representation (not a skeleton)

**Authoring:** Blender armature, 6 bones (core parent of the five). Key clips there.

**Device:** 6-wide SoA. No bind-pose inverse, no bone stack, no skinning, no blend tree.

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

Simulate in **body space** (core at origin, +Y up, yaw = 0). Apply one core 3×4 (world translation + yaw) only for projection, core-vs-cube collision, and camera focus.

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
                                      └───────→  project(pos) + sheet(view, clip, frame)
```

```c
typedef struct {
    uint8_t  id;
    uint8_t  part_mask;     // bit i set → this part has a track
    uint8_t  frame_count;   // 6–10
    uint8_t  fps;           // 15 is enough
    uint16_t duration_ms;
} ClipHdr;

// Packed after header, only masked parts:
// int16_t rest_xyz[frame_count][popcount(mask)][3];
// int16_t tip_xyz [frame_count][popcount(mask)][3];  // optional
```

Wave, 8 frames, core + arm_r, `int16`×3: **96 bytes** of rest. Tips double that.

Playback: `u = t * fps`, lerp two keys, Q8 → float, write `rest[p]`. Missing mask bits keep last emotion rest.

**Contract:** exporter samples the armature at frame *f* into `rest` **and** renders sheets at that same pose. One pose, two encodings.

**Stiffness:** while a clip is active, raise `k` on masked parts so the hand actually rises. Blend `k` down on clip end. If `k` is too low, you show a raised-arm sheet on a dangling mass.

**Gaze:** `rest_head += gaze_offset` after the clip sample. Additive. Do not replace the clip.

**No walk-cycle film in v1.** Locomotion is core translation + yaw. Legs get a tiny idle bob in the emotion table or a 2-frame clip, not 8 walk × 20 views.

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

| Pair | v1 | Why |
| :--- | :--- | :--- |
| Core vs floor / walls / toys | On | Locomotion, squish |
| Limb vs toys | On | Wave can bop a ball |
| Limb / tip vs poke ray | On | Boop hand vs nose |
| Limb vs walls / ceiling | **Off** | Don’t let a wave fight a stiff wall |
| Limb vs limb | Off | Not worth it |

Author clips **inside** the cube. Collision is not an animation editor.

**Poke:** unproject the tap through `inv(proj*view)`, ray vs spheres, closest hit. Floor ray if miss → walk / look there.

**Squish:** core penetration → uniform scale on core blit (~100 ms recover).

**Flinch:** `jerk` impulse on core + head `vel`. Springs recover into current `rest` (idle or clip).

---

## 9. Rendering

### Room

6 quads, vertex colors / tiny checker, **live** view/proj. Skip the near face (the glass). This is the “I’m looking into a cube” read. The room orbits smoothly with the IMU. Draw room first, pet after.

### Pet

Five color-key (or indexed) sprites. **One `view_idx` for all parts** (nearest dodecahedron vertex to the live camera, in pet-local space). Positions: `project(world(pos[i]))` with the **live** camera. Pixels: sheet `(part, view_idx, clip, frame)`.

True billboard: the sprite quad faces the live camera. Do not also rotate it to the bake camera — that double-applies view. The mismatch is the photo, not the quad.

```c
typedef struct {
    uint32_t pix_off;
    uint16_t w, h;
    int16_t  hot_x, hot_y;   // pivot = attach (shoulder, hip, neck)
} SpriteRec;

uint16_t sprite_id[PART_COUNT][20][CLIP_COUNT][MAX_FRAMES]; // 0xFFFF = missing
```

Missing entry → idle clip, frame 0, same part and view.

Blit: pivot at `project(world(pos[i]))`. The posed silhouette is **in the pixels**, extending from that hotspot. Do not take an idle-arm stamp and slide it to the hand.

Eyes/mouth: extra small sheets parented to `PART_HEAD`, or baked into the head sheet for v1 (simpler; blink = 2 head frames).

Toys: one colored sphere raster or one sheet.

**Dirty rect:** union of projected part AABBs + margin. ST7789 `CASET`/`RASET`. 120×140 RGB565 ≈ 7 ms @ 40 MHz.

**Budget (30 Hz, 40 MHz SPI)**

| Slice | Time |
| :--- | :--- |
| Clip + springs + camera + view_idx | < 0.5 ms |
| Room quads | < 1 ms |
| 5 sprite blits | 0.5–2 ms |
| SPI DMA | ~7–12 ms |
| Slack → light-sleep | the rest |

No runtime scale. Boom length is constant; one sprite size is enough.

---

## 10. Asset pipeline (PC)

Blender: low poly, hard edges, 16–32 colors, 6-bone armature. Origin of each part mesh at the attach.

For each clip frame *f*, for each dodecahedron vertex *v*:

1. Set armature to frame *f*.
2. Place camera at `vertex[v] * boom`, look at character origin, **same FOV and boom as runtime**. This is a *texture* camera. Runtime will look from nearby, not from here.
3. For each part: hide others, render, crop to alpha bounds, record `hot_x/y` = attach in that image.
4. Sample bone local translation → `rest` (and tip) `int16`.

**Same bake camera for every part of a (v, f).** Do not look-at each limb centroid.

Dodecahedron: vertex 0 = +Y. Store the 20 unit vectors in the blob; runtime and baker share the table for **indexing**, not for placing the live camera.

**Pixel format:** indexed-8 into the same 32-color palette as the FB. Color 0 = key.

**Size discipline**

| Content | 64×64 indexed | Fits? |
| :--- | :--- | :--- |
| 5 parts × 20 views × 1 idle | 0.4 MB | Trivial |
| + 6 emotion poses | ~2.4 MB | Yes |
| + wave 8 frames on **all** parts | ~3+ MB extra | Wasteful |
| + wave 8 frames on **core + arm_r only** | ~0.6 MB extra | Yes |

**Clip list and what plays when: TBD** (lizard brain). The pak format allows sparse clips; first art can be a single idle pose × 20 views. Extra emotions/wave are data, not engine work. Legs may reuse idle sheets (`0xFFFF` fallback).

Exporter emits one `.pak`: vertices, clips, `sprite_id`, recs, atlas, palette.

---

## 11. Lizard brain — **behaviour TBD**

The toy must run with the radio off. **What it does** (when it wanders, waves, sleeps, thinks) is not locked yet. Do not invent a personality in the engine.

**Machinery that is locked** (so behaviour can be data later):

- `emotion_rest[]` and clip tracks can drive `rest[]`
- Core 0 may set `clip_id` / `emotion` in the snapshot; Core 1 only samples and blends
- Hunger/happy **storage** in RTC exists; decay rates TBD
- Flinch is a physics impulse on `jerk` — keep that hook even if the *when* is TBD
- Gaze offset on `rest_head` is an optional hook, not a requirement

No locked mapping of poke → wave, PLUS → anything except recenter, or cortex `action` → clip. Cortex `clip_id` / `emotion` fields stay in the packet as an inbox; ignore `clip_id == 0`.

---

## 12. Optional cortex (Wi-Fi is a mode)

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
    int16_t  target_x, target_z;
    uint8_t  action;       // 0 none, 1 play, 2 eat, 3 sleep
    uint8_t  emotion;
    uint8_t  mood;
    uint16_t clip_id;      // maps to on-device clips; 0 = ignore
    uint8_t  reserved[4];
    uint16_t crc;
} server_packet_t;
```

Heartbeat 5–10 Hz while associated. Immediate ACK. LLM async; inject later. Cache by **event id**, not wrapping `seq_num`.

If/when a think pose exists: set it **immediately** on a cortex-worthy event so the LLM delay is masked. That trigger list is TBD with the lizard brain.

`.NET 10` + Ollama/Llamafile on the PC. The ESP32 does not know.

---

## 13. Power (~8 h)

1000 mAh / 8 h ≈ **125 mA average.**

| Lever | Default |
| :--- | :--- |
| Backlight PWM | ~30–40% battery; higher on USB; bump on poke, decay |
| ST7789 | Dirty only |
| CPU | 240 MHz interacting; 80/160 after a few seconds still |
| Slack | light-sleep / WFI |
| Wi-Fi | Off unless cortex |
| Audio / I2S / SD | Off |
| Deep sleep | Face-down, PWR, long idle — RTC restore |

USB in = **studio mode** (bright, 30 FPS, no light-sleep, cortex allowed). Unplug = battery personality. Always hold `BAT_EN`. PWR long-press = latch off.

---

## 14. Development

ESP-IDF ≥ 5.5. C++ as a better C: `-fno-exceptions -fno-rtti`.

`esp_lcd` ST7789 + GDMA. Own `CASET`/`RASET`. IMU = I2C FIFO + filter. CST816 on INT. Steal Waveshare **pins only**, not LVGL/XiaoZhi.

**Bring-up order**

1. `BAT_EN`, USB-CDC, octal PSRAM 80 MHz.
2. Backlight + full-screen fill. Print SPI microseconds (40 vs 80).
3. Indexed FB + dirty-rect dummy sprite.
4. Complementary filter → **smooth cube** (gravity-locked, continuous orbit). Product test.
5. One part, 20 idle sheets, blit at live `project(pos)` while `view_idx` follows the camera. Confirm the cube stays smooth and only the photo pops.
6. Six springs + one idle rest. Sphere poke.
7. Battery idle / GRAM-hold / deadband. Studio vs battery backlight.
8. UDP cortex last (inbox only). Lizard-brain policy later.

---

## 15. Checklist

- [ ] Pins vs schematic; SPI mode 0 vs 3
- [ ] `BAT_EN`; PWR latch; VBAT
- [ ] Octal PSRAM 80 MHz; quad flash 80 MHz
- [ ] Cube world-up; yaw recenter; **camera continuous**; sheet hysteresis
- [ ] Bake FOV / boom / vertices shared for **indexing**; live camera not snapped to them
- [ ] Camera deadband so idle actually skips SPI
- [ ] Pet on `Y=0`, no lean with the glass
- [ ] Indexed atlas + FB; color 0 = key
- [ ] Boom look-at stays on the pet core while it walks
- [ ] Elevation clamp; no under-floor camera
- [ ] When a clip exists: posed sheet + `rest` track + spring lag; no double transform
- [ ] Poke hits spheres, not pixels
- [ ] Dirty-rect 30 FPS; GRAM holds when idle
- [ ] Wi-Fi off: toy is whole
- [ ] ~8 h dim, radio off (measure)

---

## 16. Suggestions

1. **Do not snap the camera.** If sheet pop is harsh, hysteresis first, then optional 1-frame blend of two nearest sheets (2× blit — measure). Never quantize `cam_pos`.
2. **IMU deadband** for GRAM-hold, or 8 h dies from noise-driven 30 Hz. Tune in studio with a plot of `|Δq|`.
3. **Bake a debug overlay:** view index, clip, frame, `CCOUNT` of blit. Studio mode only.
4. **Eyes in the head sheet** until blink is worth a second atlas.
5. **Tip spheres only on arms** when a toy exists. Until then one sphere per part.
6. **Palette 32**, color 0 = key. Same palette for room expand.
7. **Exporter must fail** if runtime `dodeca_vertex[i]` and bake *index* cameras disagree by > 1e-5. The live boom is allowed to sit off-vertex.
8. **USB = studio**, unplug = battery. No settings menu in v1.
9. **Compile-time SSID** for your LAN.
10. When the kit lands: swap atlas slices, keep clips that only touch attach points (wave still works if the new arm’s hotspot is the shoulder).

---

## 17. Open questions

**Locked this pass:** look-at = pet core. Pixels = indexed-8. Camera elevation clamped (no peek-under). Lizard-brain *policy* (clips, triggers, wander/sleep/think/wave) = later.

1. **Camera `up` (still unknown).** Raw IMU `up` dutch-angles the cube when you roll the device. Orthonormalize against world +Y and tilt only changes azimuth/elevation — floor stays level, more “window on a room,” less “phone roll.” **Try orthonormalize first in studio**; keep a compile-time switch. Pick after you hold the board.
2. **Part layer order.** Z-sort `pos` vs baked `draw_order[20][PART_COUNT]`? Suggestion still: **baked order per view**; pos-sort toys only.
3. **Lizard brain** — whole policy TBD. Hunger decay, wander, gaze, sleep, think, wave, poke mappings, cortex `action` → pose. Engine keeps the hooks; do not code a personality yet.
4. **PLUS** besides recenter? TBD with the brain.

**Already locked, restated:** pet-local `view_idx`, room in live **world** view (yawing the pet must not spin the cube).

---

The body is a 30 Hz gravity-locked cube on this Waveshare. The window orbits with the IMU. The creature is five photos from 20 baked cameras, hung on six springs that chase authored rests. The lizard brain does not need a PC. The cortex is a guest.
