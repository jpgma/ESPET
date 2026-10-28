# ESPET — Architecture

**ESPET is a habitat of cube rooms. The rooms are locked to gravity. The screen is a 3/4 camera that the room sets. The device draws all of the room as live flat triangles, also when the camera does not move. The pet is one smooth-skinned mesh with six bones. The IMU pushes the contents. It does not turn the window around the room. The device operates offline first. An optional LAN cortex is available.**

Style: data-oriented C/C++ on ESP-IDF. Use POD tables and integer IDs. Do not use STL in the hot path. Do not use exceptions or RTTI. Do not use LVGL or Arduino. Core 0 simulates. Core 1 only presents. Use only the abstraction that the hardware makes necessary.

**The chip has a six-bone smooth skin.** Each vertex has two influences. Clips write bone-local transforms. The core is one spring (gravity, squish, shake, floor). The appendages are not springs. The Blender armature is the authoring tool and also the runtime skeleton.

Look references (panel pixels with nearest-neighbor scaling, not asset bakes): [refs/look/](refs/look/).

---

## 0. Product lock

| Decision | Value |
| :--- | :--- |
| Hardware | [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54) (touch and battery). For the pins, schematic and datasheets, refer to **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154/HARDWARE.md`](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) |
| SoC | ESP32-S3R8 QFN56 **rev v0.2**, dual LX7. **240 MHz is confirmed** (`CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240`; the shorter `CONFIG_ESP_DEFAULT_CPU_FREQ_240` has no effect). 512 KB SRAM, **8 MB in-package octal PSRAM** (AP Memory gen3, 3.3 V, test OK), 16 MB quad NOR |
| Display | 1.54" IPS, **240×240 visible**, ST7789, **4-wire SPI**, RGB565. Controller GRAM is **240×320**. There is **no TE and no MISO**. **Mode 3 at 80 MHz gives a clear picture** on this glass. One full frame is **12271 µs**. Use a window on the visible 240×240. The `CASET`/`RASET` origin is not locked yet. |
| IMU | **QMI8658A** (`WHO_AM_I` `0x05`, `REVISION_ID` `0x7C`), I2C **0x6B** only. INT is GPIO6, idle high. There is no magnetometer. `CTRL1` resets to `0x20` (no address auto-increment, big-endian). Before a little-endian burst, set `ADDR_AI` and clear `BE`. `CTRL2` resets to ±2 g. Before you use the **±8 g (4096 LSB/g)** scale, write it. Start the gyro at **±1024 dps** and the FIFO watermark at approximately 100 Hz. At rest with the face up, gravity is on **−Z** (approximately 1.03 g), and X/Y are less than 0.16 g. Thus **+Z points out of the glass**. Sparse play: shake, set-down, face-down. **The IMU is not a camera.** |
| Touch | **CST816D** (`ChipID` `0xB6`, proj `0x27`, fw `0x01`), I2C **0x15**. INT is 48 (idle high). RST is 47. **One finger only.** The resources pack says CST816T. The S, T and D versions have the same register map. `EnDClick` exists (`GestureID` `0x0B`). The lizard brain will set the mapping later (TBD). |
| Audio | **v1.** ES8311 at **0x18** (ID `0x83`/`0x11`) and NS4150B on GPIO7. Core 0 has one **12 kHz** play buffer. I2S DMA plays it. The impacts are procedural. The creature chirps are **s8** in flash. **Never probe** the ES7210 at **0x40**: it sends ACK, but its ID bytes stay `0xFF` until MCLK runs. The MX1.25 speaker is **installed, non-polarized and audible**. A 440 Hz sine at 16 kHz was audible. |
| Storage | A TF slot is available. **The frame loop does not use it.** Audio is not on the card. |
| Display rate | There is no locked FPS. The glass limit is **80 Hz** (**12500 µs**). A slower frame runs at its own work time. A quiet frame does not use SPI. The same limit applies to still frames and to `full_frame` clips. |
| Battery | Approximately 1000 mAh. **The 8 h target is parked.** Dirty rows and an empty SPI mask stay, so that a still pose sends nothing. A full frame is already approximately 12.3 ms of wire time. Play does not need Wi-Fi. This firmware starts Wi-Fi, so that the heap is the same as in the cortex case. |
| Look | Low-poly, hard edges, flat facets, and a silhouette similar to Spore Creatures (NDS). **Live raster of all of the room**, always. Three N·L bands, calculated at triangle setup. No photograph, no textures, no alpha, no z-buffer. Panel references: [forest](refs/look/espet_flat_forest_clearing_240.png), [night](refs/look/espet_flat_night_sleep_240.png), [play](refs/look/espet_flat_physics_play_240.png), [close-up](refs/look/espet_flat_closeup_240.png). |
| Camera | The **room sets** a 3/4 `look_at` (elevation approximately 40°, azimuth = `room.front`). FOV is approximately 55°. **Never** calculate the camera from the IMU `q`. A scripted shake or close-up sets `full_frame`. It can move the camera only for the duration of that clip. |
| Pet draw | **One** smooth-skinned mesh, six bones, **two** influences for each vertex. The camera and the room size make the pet close or far. A second mesh does not. The **shadow** is a floor stamp. |
| Pixels | **Indexed-8**, one framebuffer. Each room has a palette of **256** RGB565 entries. Index **0** is the key. Indices **1–63** are actor ramps (2–3 consecutive shades for each material; they do not change between rooms). Indices **64–255** are for the room. Expand to RGB565 only into the DMA bands. |
| Room draw | For each presented frame, draw live triangles in this sequence: sky bands, slab, props, then the skinned pet and the rigid meshes, then the shadow stamp and the FX stamps. When the camera does not move, it can keep static screen-space triangles in a cache. This is permitted only if the cache gives the same result as a full redraw. |
| Map | **S and L sizes only.** v1 rooms: Nest S, Play S, Hall L, Yard L. The kitchen is a bowl **prop** in the Hall. To eat, the pet does a core **squash**, and crumbs fall at that bowl. The flash budget is 8 rooms. Night is a palette and darker faces, not a second scene. |
| Motion | Clips write **bone locals**. FK writes the bone world matrices. The core spring writes the **core position**. Hitboxes follow the **bone joints**. Core 1 does the skinning at triangle setup. |
| Physics extras | One rigid array, with a limit of **24 awake** bodies. When the camera does not move, sleeping bodies are not in the visible scratch. The Nest contents stay quiet, so that a sleep pose can become stable. The Hall bowl stays on the floor (slide and yaw, **no flip**). |
| FX | Stamp pool of **256**, world `-Y`, floor or life only. Kinds: **dust, crumbs, leaves**. No alpha. No collision between particles. No triangle cards in v1. One SFX for each burst. |
| Brain | Offline first. **The lizard-brain *when* is TBD.** The spatial **hooks** are locked (Nest clips, Hall/Yard wander, Play toys, door swap, Hall eat squash, shake → dust/leaves). |
| Parts kit | **Not in v1.** The skeleton has six bones. A later kit replaces the weighted mesh. It does not change the bone count. |

**Golden rules**

1. Core 1 never waits for Core 0, Wi-Fi or the LLM. If the sim stops, Core 1 holds the last pose.
2. The pet is complete when the radio is off.
3. World down is real gravity. The screen is a camera, not a world axis.
4. A still pose does not send pixels. If the CPU runs at 240 MHz with a static frame, that is a bug. **Silence with the I2S clocks on or the PA on is the same bug.** The 8 h target is for later. The empty mask is the method that lets a still pose send nothing. The present limit is the glass, **12500 µs**.
5. Meshes are appearance. The core spring is state. Bone clips are appearance. Pixels are not physics.
6. Clips write **bone locals**. The core spring only adds lag to the body. The appendages do not spring.
7. The **room sets** the camera. The camera stays static in each room until a clip sets `full_frame`. IMU tilt does not turn the camera around the room.
8. Core 1 never plays audio. The sim on Core 0 renders one-shots into the play buffer. I2S DMA plays it.

The raster core that this product uses (filler, clip, pose mailbox) is the task list in [`docs/raster`](docs/raster/README.md). This firmware starts Wi-Fi, so that the internal heap is the same as in the cortex case. The pet is complete when the radio is off.

---

## 1. Gravity-locked habitat, authored camera

The rooms are cubes on the world axes. They are attached to the Earth. The pet stands on the floor, with its feet toward real gravity. The physical screen is a **3/4 window on the current room**. You cannot turn it around the room when you tilt the device. The pet does **not** lean with the glass.

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

The pet height is `1.0` world unit.

### Two play modes

| | **Close (S)** | **Habitat (L)** |
| :--- | :--- | :--- |
| Floor | Approximately 2×2 pet lengths | Approximately 8×8 pet lengths |
| Camera | Shows the **pet** | Shows the **room AABB** |
| Pet on the glass | Approximately 80–100 px | Approximately 24–32 px |
| Draw | The same skinned mesh | The same skinned mesh, smaller in the frame |
| Tap | Bone-joint spheres | Screen-space tolerance of 24 px minimum (pet, toys, props) |
| Use | Wave, poke, sleep, blink | Wander, doors, IMU bounce, eat at the bowl |
| Dirty rect | Approximately 120×140 | Approximately 32×32 for the pet and moving items; FX can make it larger |

L rooms have no follow camera. A door is a smash cut (or a full-frame slide of approximately 200 ms). A close-up is a **camera clip**. It is not a room and not a second mesh.

The inspiration sheet shows the look, not the map. The Yard looks like the forest clearing. The Nest sleep looks like the night slab. Play looks like the pet and a ball.

### v1 map

```
Nest S  ←→  Hall L  ←→  Yard L
              ↕
           Play S
```

| Room | Size | Function |
| :--- | :--- | :--- |
| **Nest** | S | Sleep, poke, close-up clips. The contents stay quiet, so that a sleep pose can become stable. |
| **Play** | S | Toys; limb joints against the ball |
| **Hall** | L | Wander, doors, bowl prop (the eat magnet → core squash and crumbs). The bowl **slides and yaws**. It does not flip. |
| **Yard** | L | Wander, scenery, toys, IMU bounce, leaf FX on shake |

**Limits for each resident room**

| Item | Limit |
| :--- | ---: |
| Scenery AABBs | 24 |
| Props (tap magnets; they can also be rigid) | 8 |
| Awake rigid bodies | **24** |
| Doors | 4 (one for each wall; a wall can have none) |
| Visible triangles | **1024** (room, pet and moving items) |
| Event FX stamps | 256 |

The sky is a small number of horizontal quads. A floor is a small number of large triangles, not a grass texture. For static contact darkening, the author makes the faces below the props darker. The shadow of the moving pet is a floor stamp.

There is one resident room. More rooms later are pak data, not engine work. The flash budget is **8** rooms. Night is a palette and darker faces, not a second bake.

### Habitat shots

Art targets (engine hooks, not lizard *when*):

1. Hall: the skinned pet walks behind a plant mesh. **The shadow stays on the floor.**
2. Smash cut from Hall to Nest (door).
3. Nest: a sleep clip on the bones. The core spring becomes stable.
4. Hall bowl: **core squash**, a burst of crumbs and one impact. No kitchen. The bowl does not flip.
5. Yard shake: leaves fall along real `-Y`. A prop can tip. A scripted camera shake can set `full_frame` for that burst.

Bring-up on silicon starts with a live Nest slab and a bounce (§15). Do not go directly to these shots.

### 6-axis IMU: sense always, play sparsely

The QMI8658 measures gravity (accelerometer) and angular velocity (gyro). This gives **pitch and roll** fully. It cannot measure **yaw around gravity**. There is no magnetometer.

Core 0 runs the complementary filter at 100 Hz. It does **not** make a camera from `q`. Core 0 classifies the events and sends one-shots to the sim, which is also on Core 0.

| Event | Detection | Play | Rate limit |
| :--- | :--- | :--- | :--- |
| Idle | `\|ω\|` and `\|jerk\|` are less than epsilon | Nothing | — |
| Shake | `jerk` is more than the threshold | Impulse on the core spring. Awake rigids get the impulse too. **Dust** at the feet. The Yard can emit **leaves**. A clip can also set a `full_frame` camera shake. | Cooldown of approximately 200 ms, approximately 5 Hz |
| Set-down | A spike, then no movement | One startle | Edge |
| Face-down | Existing | No new sound. Wait for TX-done. Sleep. | Edge |
| Held tilt | — | **Off in v1** | — |

**Tilt does not change the world gravity.** The core spring uses `-Y`. A bounce is `vel`, not a snow globe.

A short push on PLUS and a CST816 double-tap are **one-shots**. They do **not** recenter the yaw. The lizard brain will set the mapping (TBD). They do not control the volume.

### Authored camera, optional full-frame clip

Each room keeps a baked `view` and `proj` (or sufficient data to make the `look_at` again). Blender and the runtime use the same FOV.

```
view        = room.view                 // 3/4; elev ~40°; azimuth = room.front
proj        = room.proj                 // perspective ~55°, aspect 1.0
light_dir   = room.light_dir            // world-space unit; Nest window, Yard sun
full_frame  = clip wants a moving camera // shake or close-up; else 0
```

A clip that sets `full_frame` can replace `view` for its duration (a damped shake, or a dolly to the pet). When the clip stops, the room view comes back. If the camera moves by more than one pixel, all 240 rows become dirty. In v1, IMU pitch and roll do **not** control the camera.

**Idle present:** if the interpolated pose is the same as the last presented pose (bones, rigids, FX, camera), Core 1 does not use SPI. There is **no** `|Δq|` camera test. When the camera does not move, Core 1 can keep the static triangles in screen-space scratch and fill only the dirty window again. That cache must give the same result as a full redraw of those triangles. Rigid-body **sleep** lets the mask become empty.

---

## 2. Hardware map

For the full GPIO table, the schematic and the strapping traps, refer to [jpgma/esp32-s3 HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md) (private). Compare this table with that schematic one time. Then use it as the rule.

This is a short lock. Do not invent pins. **Set `BAT_EN` high in `app_main` before all blocking work.** The SPI host is a mux. This product uses **SPI3**. The vendor demos use SPI2, but SPI2 is not a net.

| Function | GPIO | Notes |
| :--- | :--- | :--- |
| LCD SPI3 (CS/CLK/MOSI, DC/RST/BL) | 21/38/39, 45/40/46 | Write only, **no TE and no MISO**. 45 and 46 are strapping pins. |
| I2C (SCL/SDA) | 41/42 | One bus, one owner. The pads use these same wires. |
| IMU INT | 6 | FIFO watermark. This is the product path. The hello poll is only for the testbed. |
| Touch INT / RST | 48 / 47 | IRQ. Do not poll it from the IMU drain. |
| PA (NS4150B CTRL) | 7 | Gate it between phrases. |
| I2S MCLK/BCLK/WS/DIN/DOUT | 8/9/10/11/12 | DIN = ES7210 (not used). DOUT = ES8311. |
| BAT_EN | 2 | **Hold it high** (`rtc_gpio_hold` in deep sleep). If not, the board stops on battery power. |
| VBAT ADC / CHG_STAT | 1 / 3 | VBAT = VADC **×3**. `CHG_STAT` is **active-low** during charge. GPIO3 is also an S3 strapping pin. |
| PWR / PLUS / BOOT | 5 / 4 / 0 | A long push on PWR latches the power off. PLUS is for the lizard brain (TBD). |
| USB D−/D+ | 19/20 | Native CDC / JTAG. There is no CH340. Serial/JTAG **stops in deep sleep**. |

I2C 7-bit addresses: QMI8658 **0x6B**, CST816 **0x15**, ES8311 **0x18**, ES7210 **0x40 (never probe)**. There is no dedicated RTC IC. For persistence, use the S3 RTC SRAM. The TF SDMMC 4-bit pins (16/15/17/18/13/14) are not used. **GPIO18 is TF D1. It is not a spare pin.** The ISET resistor (**160 kΩ**) sets the charge current. Software does not set it.

**sdkconfig:** `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y`, `CONFIG_SPIRAM_MODE_OCT=y`, `CONFIG_SPIRAM_SPEED_80M=y`, quad flash at 80 MHz (not OPI), `CONFIG_FREERTOS_HZ=1000`, Bluetooth **off**, Wi-Fi **started** (the heap test is the cortex case; play is complete with Wi-Fi off), USB CDC on boot. The flash JEDEC ID of this unit is **0x204018** (XMC). The schematic text says W25Q128JVSIQ. IDF reports the chip as `generic`. QIO at 80 MHz already boots.

**SPI:** **mode 3, 80 MHz**. The picture is clear. A full 240×240 RGB565 frame was measured at **12271 µs** (wire time is approximately 11.5 ms; at 40 MHz it is approximately 23 ms). Dirty rows stay, because a full frame is already approximately 12.3 ms of wire time and a still pose must not send pixels. A `full_frame` clip or a door sends the full frame. The controller GRAM is **240×320**. Use a window on the visible **240×240**. Lock the offsets before you trust the dirty rect.

---

## 3. Memory

Wi-Fi DMA cannot be in PSRAM. The plan is one indexed frame and the radio. There is no reserved second indexed frame. There is no reserved 16-bit z-buffer (115 KB).

| Region | Use |
| :--- | :--- |
| **Internal DRAM** | One **8-bit indexed** framebuffer (57.6 KB). Palette of 256×RGB565 (512 B). Two DMA bands of 8 rows: `2 × 8 × 240 × 2 = 7680` B. Screen-space triangle scratch for 1024 triangles (approximately 32 KB). Pose mailbox with three slots (camera, six bone matrices, rigid instances). Core spring, `Rigid[24]`, `FxPool` 256, seqlock, **play buffer** (12 kHz × 300 ms `int16` = 7200 B), RTOS stacks. The raster inner loop **never** reads PSRAM. The play buffer stays in DRAM. Playback does not read PSRAM or flash. No z-buffer. No RGB framebuffer. |
| **Octal PSRAM** | Optional cold copies (prefetch of the next room mesh, stamp pixels if XIP thrashes). Not the framebuffer. Not the play buffer. Not the pose. Not a backdrop. |
| **16 MB flash** | Firmware, bone clips, weighted pet mesh, room meshes, `MatRamp[]`, room records, **palettes for each room** and `light_dir`, FX/shadow stamps, `ImpactPatch[]`, creature chirps (**s8**). Use XIP for cold tables and meshes. The sim reads the chirp bytes one time when it renders. |
| **RTC SRAM** | Hunger, happy, sleep, last emotion, **`room_id`**. |

**Scanout:** indexed frame → the palette expands the dirty rows into a DMA band → GDMA sends it to the ST7789. There are two bands, so that the expansion of band *k+1* occurs at the same time as the DMA of band *k*.

**Budget**

| Block | Location | Size |
| :--- | :--- | :--- |
| Indexed frame | DRAM | 57.6 KB |
| Palette | DRAM | 512 B |
| DMA bands | DRAM | 7.7 KB |
| Triangle scratch (1024) | DRAM | Approximately 32 KB |
| Pose × 3 | DRAM | Approximately 2–4 KB |
| Core spring + `Rigid[24]` | DRAM | Approximately 2 KB |
| `FxPool` 256 | DRAM | Approximately 8 KB |
| Play buffer | DRAM | 7.2 KB |
| Pet mesh and weights | flash XIP | A few KB |
| Room meshes, 4 rooms | flash XIP | Tens of KB |
| Palettes | flash | 4 × 512 B |
| Shadow and FX stamps | flash | A few KB |
| Creature chirps (s8) | flash | A few KB |
| Firmware | flash | Approximately 1–1.5 MB |

There is no 57.6 KB backdrop. There are no room photographs of 230 KB.

---

## 4. Core allocation

### Core 0 — sensors, sim, optional radio

| Priority | Job | Rate |
| :--- | :--- | :--- |
| IDF (approximately 22) | Wi-Fi / LwIP | Cortex mode only |
| 13 | **I2C owner** (the only task that uses 41/42) | Drain / event / codec posts |
| 12 | Complementary filter and **IMU event classification** (after the owner drain) | 100 Hz |
| 8 | Needs, wander, gaze, flinch, clip requests, room walk, cortex inject | 20 Hz |
| 6 | **Sim:** core spring, FK, rigid bodies, particles, collision, render one-shots, publish the pose | 30 Hz |
| 5 | Backlight, VBAT, BAT_EN, PWR, Wi-Fi up/down | 1–10 Hz |

Playback uses DMA. Thus, a long solve does not stop I2S. **One I2C owner** on 41/42 is the **only** task that calls I2C. The GPIO6 (IMU FIFO) and GPIO48 (CST816) ISRs **post** only. Do not use I2C in an ISR. The owner drains the QMI8658 FIFO and reads a CST816 burst. It never writes the ES8311 during that drain. The sim never uses I2C. It posts the codec start, stop and volume. Do not add the ES7210 to the device table. The owner puts the bus operations in sequence.

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

If a frame is not complete when the next tick arrives, count one missed deadline and complete the frame. Do not start a second frame at the same time.

Lock the timing with `CCOUNT` or the gpTimer. Core 1 does not step the physics. It does not read the IMU and does not use I2C.

On a door: Core 0 loads the meshes and palette of the next room. It publishes a pose with `full_frame`. It puts the pet on the opposite face.

---

## 5. Pose mailbox (DRAM only)

The mailbox has three slots. The publisher never waits for the present. The renderer never waits for the publisher. On Xtensa SMP, `volatile` is not a barrier.

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

Core 1 loads the latest published slot and the slot before it. It interpolates to the **frame deadline**, not to the time of the last sim step. If the newest timestamp is older than one period, Core 1 draws that pose without a change. Do not extrapolate.

Core 1 rejects a torn slot (with a seqlock, or with a sentinel word that the publisher writes first and last).

### Sound

The sim renders the sound in place: a maximum of one impact and one chirp on the tick that made them. Refer to [§9](#9-audio). There is no ring.

The UDP packets are packed little-endian. They are **not** `Pose`. The cortex `target_x/z` is **in the room**.

---

## 6. Runtime representation

**Authoring:** a Blender armature with 6 bones (the core is the parent of the other five). Make the clip keys in Blender. Export one weighted mesh (a maximum of two influences) and the bone-local keys.

**Device:** the core is one spring. The other five bones get FK from the clip, with an optional one-pole jiggle on the local rotation. There are no appendage springs. There is no blend tree. There is no IK.

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

Simulate the core in **body space** (origin, +Y up, yaw = 0). Then one core 3×4 matrix (world translation, yaw and squash) is the parent of the skeleton.

Core 0 does **not** do the skinning. Core 1 does it for each triangle: it mixes the two bone matrices onto each of the three vertices, and then clips and fills the triangle. There is no deformed vertex buffer.

---

## 7. Clips: animation writes bone locals

A wave is a **bone-local rotation** on the arm. The mesh bends, because its vertices use that bone and a neighbor bone. The core spring is separate. A clip can also move `core.rest`.

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

A wave of 8 frames on the core and the arm is a few hundred bytes. It is not a posed sheet.

Playback: `u = t * fps`. Lerp the keys. Write `local_q` and `local_t`. If a bit is not in the mask, that bone keeps its bind local. Then do FK.

**Gaze:** an additive local rotation on `BONE_HEAD`, after the clip sample. Only in S rooms. Do not replace the clip.

**Jiggle:** an optional one-pole filter on a bone local, after the sample. This gives the shake of a leaf or an ear. It is not a spring and it does not collide.

**Vox:** if `vox_id != 0`, Core 0 renders that chirp when the clip **starts**.

**Blink:** use a second set of face weights. Or, if you add a lid bone later, use a clip that turns it. In v1, the lid is two influences on the eye area of the same mesh, or a clip. Do not add a face stamp. L rooms also show the face, but it is small.

---

## 8. Physics and hitboxes

```
force = (rest - pos) * k - vel * d
```

That equation is for the **core only**. The limbs do not integrate.

**Hitboxes are spheres on the bone joints. Never use sprite alpha. Never use mesh triangles.**

```
hitbox[i].c = bone[i].joint
hitbox[i].r = bone[i].radius
```

**Rigid bodies** (toys and props) have position, orientation, linear and angular velocity, mass, inertia and a collider (sphere or box). Core 0 runs an impulse solver at 30 Hz. A body goes to **sleep** when `|v|` and `|ω|` stay less than epsilon. An impulse, a shake or a poke wakes it. The limit is **24 awake**. When the camera does not move, the pose does not include sleeping bodies.

Hall **bowl:** it stays on the floor. It slides on `y=0`. Its yaw is free. It has **no pitch and no roll**.

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

| Pair | S | L | Reason |
| :--- | :--- | :--- | :--- |
| Core against floor / walls | On | On | Locomotion, squish |
| Core against scenery AABB | On | On | Walk around furniture |
| Core against rigids | On | On | Bounce |
| Rigid against scenery AABB | On | On | Ball below a table |
| Rigid against rigid | On | On | Sandbox; this is the expensive pair |
| Arm joints against toys | **On** (Play) | **Off** | A wave hits a ball |
| Joint against poke | **On** | **Off** | Boop |
| Joint against walls / scenery | Off | Off | Do not fight a clip |
| FX against floor `y=0` | On | On | Dust, leaves and crumbs land |
| FX against scenery / pet / toys | Off | Off | Not worth the cost |

A pile of stacked boxes is the frame that costs milliseconds. Free bodies on the floor are cheap. If the solve misses a publish, Core 1 holds the last pose.

**Poke**

- **S:** unproject the tap through `inv(proj*view)`. Test the ray against the joint spheres and use the nearest hit. If there is no hit, use a floor ray: the pet walks or looks there.
- **L:** use a **screen-space** pick. Make the projected AABBs larger, to a minimum of **24 px**. Use the nearest of the pet, toys and props. If there is no hit, the pet walks to that point on the floor.

**Squish:** penetration of the core → non-uniform `core.scale` (recovery in approximately 100 ms). The Hall eat uses the same path. If the closing speed is more than the threshold, render an impact.

**Shake:** an impulse on the core and on the awake rigids. Spawn **dust** (and **leaves** if the room has an emitter). A clip can also set `full_frame`. One SFX for each FX burst.

**Eat (Hall):** the bowl is a **prop** magnet. When Core 0 requests eat: core squash, a **crumb** burst and **one** impact. The *when* is for the lizard brain (TBD).

**Door:** core against door volume → change `room_id`. Load the meshes **and the palette** and `light_dir`. Put the pet on the opposite face. Set `full_frame` one time.

---

## 9. Audio

**Locked:** v1. The sim renders one-shots into one buffer. I2S GDMA plays it. There is no mixer task. The ES7210 is off. The PA is gated. Let DMA complete before the chip goes to sleep. The 8 h target is parked; the chirps are not.

```
Core 0 collide / squish          → render impact   // noise + one-pole, vel scales decay
Core 0 clip start (vox_id)       → render chirp    // s8 from flash, expand to int16
Core 0 shake / jerk              → render chirp

play[3600] int16                 // 12 kHz, 300 ms, 7200 B, internal DRAM, DMA
TX-done ISR                      → stop I2S clocks
then                             → PA low
```

**PA times (NS4150B typical):** set GPIO7 high a **minimum of 35 ms** before the first sample (wake). For a cold start, set it high a **minimum of 120 ms** before the first chirp. The render time is less than this wait. Do not trigger again during the shutdown of approximately **80 ms**. With CTRL=0, `ISD` is 1–10 µA. The PA **must** go low between phrases.

**12 kHz, not 24 kHz.** This speaker and this look do not need the 24 kHz duplex of XiaoZhi. A 440 Hz sine at 16 kHz was already audible on this MX1.25 speaker. Thus, the amplifier and the coil operate. The product rate stays at 12 kHz. Use the **pins** from XiaoZhi, not its audio graph. No MP3, no TF, no AEC.

**Two classes, one buffer.** An impact is a noise burst with a one-pole envelope. A creature sound is a baked s8 clip (effort, yelp, happy, sleepy), expanded with a gain. When a wave hits a toy, the two classes are layered: the second class **adds** into the buffer in front of the DMA read pointer. A **32-sample** guard keeps the write in front of the samples that DMA already read. The buffer is in internal DRAM, so the add is a usual saturating add. The last event of a class overwrites from the play cursor forward, with a crossfade of **5 ms** (60 samples). A maximum of one render occurs for each class in each sim tick. Idle ambient loops are **off**.

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

**Sleep:** Core 1 can skip SPI while DMA continues to read. Do **not** put the chip into light sleep or deep sleep, and do not set the PA low, until TX-done occurs and the PA shutdown is complete. The buffer length is the limit (300 ms). Face-down: **do not start** new events. Let DMA complete. Set the PA low. Then sleep.

**Codec:** initialize the ES8311 on the first sound (I2C **0x18**, owner task). Put it in **standby** between phrases. (A new initialization takes tens of ms, and the sound of a wall hit would come too late.) Turn it fully off only for deep sleep. In v1, the volume is **fixed** (studio can be louder, as for the backlight). PLUS does not control the volume. The sim posts start, stop and volume. It does not write the codec.

**Speaker:** MX1.25 header, non-polarized. A 440 Hz sine at 16 kHz was already audible. Thus, the amplifier and the coil operate.

**Mic / ES7210:** I2C **0x40**. Two MEMS microphones and an analog AEC tap on MIC3. **Never probe it. Never initialize it.** The DIN pin is not used.

---

## 10. Rendering

### Room

Each room has one authored VP, unless a clip sets `full_frame`. Draw in this sequence, from far to near:

```
sky bands + slab + static props     // live triangles
skinned pet + rigid meshes          // skin at triangle setup, then the same filler
shadow stamp                        // floor, at the projected core on y=0
FX stamps                           // dust / crumbs / leaves; no alpha; on top
```

**There is no backdrop photograph.** When the camera does not move, Core 1 can use the screen-space triangles of static meshes again. That cache is an optimization. It must give the same result as a redraw. When the camera moves, delete the cache and set all rows.

**Palette:** on a door, copy the 256 RGB565 entries of the room. Index 0 is the key. **1–63** are the actor ramps. **64–255** are for this room. Scanout uses that one table.

**Light:** `room.light_dir` is a world-space unit vector. At **triangle setup** (after skinning, not in the pixel loop): `band = quantize(N·L, 3)` → write `mat.ramp[band]`. No textures. No Gouraud. No N·L for each pixel.

**Smooth skin** bends the elbows. This gives a softer silhouette than rigid parts. It is the selected look. It does not add a texture or a gradient in a triangle: each triangle is one flat index.

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

Raster one triangle at a time. For a skinned triangle, mix the two bone 3×4 matrices onto each vertex. Then clip and fill. The filler writes a palette index. It does not use floats.

**Triangle budget (visible, for each frame)**

| Set | Triangles |
| :--- | ---: |
| Room shell and props | A few hundred |
| Pet | A few hundred vertices, much less than approximately 200 triangles |
| Awake rigids | The remaining triangles, up to the limit |
| **Limit** | **1024** |

The fill, not the triangle count, is the limit. A still S window (approximately 120×140, approximately 2× overdraw) takes approximately 1–2.5 ms. A `full_frame` clip at approximately 2× over all of the glass takes approximately 4–10 ms. It overlaps the 23 ms wire time at 40 MHz. With approximately 4× overdraw on a moving camera, the frame misses the glass limit. Measure it. If the work is slower, that frame runs at its work time. Do not add a second cadence.

### FX

Use stamps, not cards. DRAM SoA. World space. Gravity is `-Y`. A particle stops when `life` ends. No collision between particles. Floor only.

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

Spawn: shake → dust (leaves if the room has an emitter). Floor impulse → dust. Eat → crumbs. One impact for each burst.

`fx_live` is true if any `life[i] != 0`. An empty mask needs a matching pose: the camera does not move, the bones do not move, the rigids sleep, and `fx_live==0`.

**Dirty rows:** the union of the projected AABBs of the moving items, one bit for each row. `full_frame` sets all 240 rows. Set the ST7789 `CASET`/`RASET` for each band. Doors and `full_frame` clips send the full 240×240 frame: approximately 23 ms at 40 MHz, approximately 11.5 ms at 80 MHz. Tearing is accepted (there is no TE pin).

**Budget (glass limit, 80 MHz SPI)**

| Slice | Still camera | `full_frame` clip |
| :--- | ---: | ---: |
| Core spring, FK, rigids, FX (Core 0) | Usually 1–4 ms; a stacked pile can take more | The same; a miss holds the pose |
| Skin and raster (Core 1) | Dirty window, approximately 1–3 ms | Approximately 4–10 ms at approximately 2× |
| SPI DMA | Dirty rows | **Approximately 12.3 ms** at 80 MHz, at the same time as the filler |
| Slack | Only if the frame is complete in 12500 µs | The wire is the longest part |

`board-sim` never rasterizes a habitat. The rasterizer is in `firmware/`. Its output shows as ST7789 GRAM pixels.

---

## 11. Asset pipeline (PC)

Blender: low poly, hard edges, flat faces, a 6-bone armature. A material ID on each vertex. Each vertex has two bone indices and two weights. The sum of the weights is 1. For a rigid vertex, set the second weight to zero.

**Room:** meshes, not a render. Sky quads, a slab of a small number of triangles, and props as separate meshes. Keep `light_dir` and the palette of 256 entries. The author makes the faces below static props darker. Do not paint a 240×240 photograph.

**Pet export:**

1. Put the armature at bind.
2. Export one mesh, with the origin at the core and a maximum of two influences.
3. The **shadow** is a separate floor stamp.
4. Clip frames: export the bone-local rotation (and the optional translation) as `int16`, one time for each frame. Do not export a mesh for each frame.
5. If the clip moves the camera, set `full_frame` in the clip header.

**The exporter must fail** if a vertex has more than two influences, if a material ramp uses indices out of 1–63, or if the visible set of a room is more than 1024 triangles.

Rooms in the pak: ID, size (S/L), front, view/proj, `light_dir`, `palette[256]`, scenery AABBs, props, doors, mesh IDs, toy slots, optional leaf emitter.

```
impact_count, ImpactPatch[impact_count]
chirp_count, ChirpHdr[chirp_count] + s8 samples
```

---

## 12. Lizard brain — **behavior TBD**

The toy must operate with the radio off. **Its behavior** is not locked yet.

**This machinery is locked:**

- Bone clips and `core.rest` can control the pose.
- Core 0 owns the sim. It can set `clip_id`, `emotion`, `walk_x/z` and `room_id` before it publishes.
- The RTC **storage** for hunger and happy exists. The decay rates are TBD.
- Nest S: close-up clips and joint poke. The contents stay quiet.
- Play S: toys; arm joints against toys.
- Hall / Yard L: steer around the scenery AABBs; rigids from the shared limit of 24 awake.
- The bowl in the Hall is an eat **prop** and a rigid that stays on the floor. Core 0 can request squash and crumbs. No kitchen.
- Door volume → change the room (meshes, palette and `light_dir`).
- A shake is a core impulse and dust (leaves if the room has an emitter). A clip can add `full_frame`.
- A gaze offset on the head bone is an optional hook (S).
- `ClipHdr.vox_id` renders a chirp when the clip starts.
- `collide()` can render an impact.

There is no locked mapping of poke → wave, PLUS → clip, or cortex `action` → pose. The cortex `clip_id` and `emotion` stay in the packet. `target_x/z` is in the current room.

---

## 13. Optional cortex (Wi-Fi is a mode)

The radio is **off** by default. Use STA mode when the board has USB power, or when a "brain" flag is persistent and a known AP is available. Do not try to connect again and again. If the connection fails, use the lizard brain.

Do not use JSON or HTTP in the frame path. Use UDP, little-endian, LAN, port 8888.

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

While the board is associated, send a heartbeat at 5–10 Hz. Send the ACK immediately. The LLM is asynchronous. Inject its result later. Use the **event ID** as the cache key, not the `seq_num` (it wraps).

If a think pose is added: set it **immediately** on an event for the cortex. This hides the LLM delay. The trigger list is TBD with the lizard brain.

The PC uses `.NET 10` with Ollama or Llamafile. The ESP32 does not know about this.

---

## 14. Power (8 h parked)

1000 mAh / 8 h ≈ **125 mA mean.** This target is **not** locked in this pass. The frame period is locked. The chirps stay. Idle I2S stays off.

The panel LED is usually **60 mA at 3.0 V**. At 100%, that is approximately half of 125 mA. Thus, when the 8 h target comes back, a battery PWM of **30–40%** is necessary. Start from the vendor setting: **LEDC 5 kHz / 10-bit**. `CHG_STAT` is a status. It does not change the play mode. The ISET resistor sets the charge current. The firmware does not set it.

A still pose does not use SPI because the pose is the same, not because of the battery. A full frame was measured at **12271 µs** at SPI mode 3, 80 MHz. `full_frame` clips and doors send that full frame on purpose.

| Lever | Now | Later, if the 8 h target comes back |
| :--- | :--- | :--- |
| ST7789 | Dirty rows; an empty mask when the pose is the same | The same |
| CPU | 240 MHz while a frame has work; wait only for the remaining glass period (12500 µs) | 80/160 MHz after a few seconds with no movement |
| Slack | Core 1 waits for the deadline | Light sleep only if DMA is idle and the PA is down |
| Wi-Fi | This firmware starts it, so that the heap is the same as in the cortex case. Play is complete with it off. | The same |
| Audio | PA and I2S clocks are on only while DMA runs. ES7210 off. SD off. | The same |
| Backlight | On for bring-up. USB can be brighter. | **30–40%** on battery (60 mA LED) |
| Deep sleep | Face-down and PWR also need TX-done, PA low and `rtc_gpio_hold` on `BAT_EN` | The same |

USB connected = **studio mode** (bright, glass limit, cortex permitted, USB debug). When you disconnect USB, the raster does not change. Always hold `BAT_EN`. A long push on PWR latches the power off. In deep sleep, USB Serial/JTAG is **not** available.

The desk card is a visit on the host while the cable is connected. It is not a room. Refer to [docs/studio.md](docs/studio.md).

**v1 wake map** (confirm the edges on silicon):

- **Light sleep** (DMA idle, PA down): timer, CST816 INT **48** and PLUS **4**. Use IMU INT **6** only if a shake must stop the sleep. USB debug is off in this state.
- **Deep sleep / PWR latch-off:** hardware (PWR or a USB connection), not a software IMU camera. The face-down *pose* is for the lizard brain (TBD).

---

## 15. Development

ESP-IDF 5.5 or higher. Use C++ as a better C: `-fno-exceptions -fno-rtti`.

For pins, schematic and silicon procedures, refer to **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). The raster track there is the filler and the pose mailbox. Use `esp_lcd` ST7789 with GDMA. Set your own `CASET`/`RASET` (the visible 240×240 of the 240×320 GRAM). IMU = I2C FIFO and GPIO6 (polling is only for hello and the testbed). CST816 on INT 48. I2S TX to the ES8311. Use only the Waveshare **pins**. Do not use LVGL or XiaoZhi (not the 24 kHz duplex/AEC stack).

**Bring-up sequence**

1. Set `BAT_EN` **first** in `app_main` (hello must drive GPIO2). Then USB-CDC and octal PSRAM at 80 MHz.
2. Backlight and a full-screen fill. Print the SPI microseconds (40 MHz against 80 MHz). If the glass is clean at 80 MHz, use 80 MHz.
3. One indexed FB, two DMA bands and dirty rows. An empty mask does not call SPI.
4. Complementary filter → gravity, face-down and `jerk`. A **live Nest slab** (a small number of triangles, not a photograph), with Earth up. **Product test:** tilt does **not** turn the camera; a shake makes the debug core jump.
5. One weighted mesh, six bones, two influences, flat N·L. A debug clip turns a bone. The room does not move.
6. Core spring and FK. Joint poke (S). A small number of rigid toys. Door cut Nest ↔ Hall, with `full_frame` one time.
7. Pose mailbox: Core 0 publishes and Core 1 interpolates. A sim that stops for short periods does not move the deadline. An empty mask when the pose is the same.
8. Audio: ES8311 I2C, a PA pulse, a 12 kHz sine, PA low. Then a collision impact and one clip with `vox_id` render into the play buffer. TX-done stops the clocks. The PA goes low. Face-down waits until DMA is idle.
9. UDP cortex last (inbox only). The lizard-brain policy comes later.

---

## 16. Checklist

- [x] Pins compared with [HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md). SPI **mode 3 at 80 MHz**, clear image, frame 12271 µs
- [ ] `CASET`/`RASET` origin
- [ ] `BAT_EN` first; PWR latch; VBAT ×3; `CHG_STAT` active-low
- [x] Octal PSRAM at 80 MHz (AP gen3, test OK); quad flash at 80 MHz (JEDEC `0x204018`)
- [ ] Cube world up; pet on `Y=0`; the pet does not lean with the glass
- [ ] The room sets the camera; tilt does not turn it; `full_frame` is a clip flag
- [ ] One indexed FB; two DMA bands; palette of 256; 0 = key; 1–63 actor ramps; 64–255 room
- [ ] No backdrop photograph; the sky is quads; the floor is a small number of large triangles
- [ ] Six-bone smooth skin, two influences; skinning at triangle setup; the core is the only spring
- [ ] Pose mailbox with three slots; Core 1 interpolates to the deadline; if the sim stops, Core 1 holds the last pose
- [ ] Visible triangles ≤ 1024; the scratch stays internal
- [ ] The shadow is a floor stamp; N·L at setup; no textures, no z-buffer
- [ ] Awake rigids ≤ 24; the bowl does not flip; the Nest contents stay quiet
- [ ] `FxPool` of 256 stamps; dust/crumbs/leaves; floor only; one SFX for each burst
- [ ] An empty mask when the pose is the same; `full_frame` sends all 240 rows
- [ ] Hall eat = core squash, crumbs and one impact; no kitchen
- [ ] One I2C owner; QMI8658 0x6B FIFO; CST816 0x15 on INT 48; ES8311 0x18; ES7210 never probed
- [ ] PA wake ≥35 ms, cold ≥120 ms; 12 kHz sine; ES7210 off
- [ ] A collision renders an impact; a clip renders a chirp; TX-done before sleep
- [ ] Light-sleep wake: timer, INT 48 and PLUS 4 (IMU 6 only for shake-to-wake)
- [ ] GPIO18 is not a spare pin
- [ ] Wi-Fi off: the toy is complete
- [ ] The 8 h target is parked, not a gate
- [ ] The rasterizer is in the firmware; `board-sim` never rasterizes a habitat

---

## 17. Recommendations

1. Never use the IMU to turn the camera. If a shake must move the glass, use a `full_frame` clip, not `q`.
2. The **empty mask** means "the pose is the same". It does not mean `|Δq|`. Show the count of awake bodies in the overlay. A prop that jitters keeps the mask active.
3. **Debug overlay:** room ID, clip, frame, awake rigids, `full_frame`, raster `CCOUNT`, VBAT ×3, backlight PWM. Studio only.
4. **Blink** stays on the skinned mesh. Do not add a face stamp.
5. Use a **palette of 256**, with the actor ramps in 1–63. Thus, a night cap keeps its red.
6. **The exporter fails** on a third influence, on a ramp out of 1–63, or on a room with more than 1024 triangles.
7. **USB = studio.** When you disconnect it, the raster does not change. There is no settings menu in v1.
8. Use a **compile-time SSID** for your LAN.
9. A later kit replaces the weighted mesh and keeps the six bones. Thus, a wave clip still moves the arm.
10. **Gate the PA and I2S.** Silence with MCLK on is the audio version of a still frame that sends SPI. PA wake ≥35 ms, cold ≥120 ms. Do not trigger again during the shutdown of approximately 80 ms.
11. The thuds stay procedural, so that the velocity can change the decay. The creature lines are already s8 chirps in the same buffer. Do not add MP3 or the TF slot.
12. Optional later: an IMU parallax of a maximum of 3°. **v1 = 0.** That parallax would set `full_frame` on each tick. Do not turn it on without a good reason.
13. **Calculate N·L at setup, not in the pixel loop.** The flat bands are the look and the budget. Index-Gouraud stays parked.
14. **Leaves as 2-triangle cards** can wait. v1 uses stamps.
15. **SPI mode 3 at 80 MHz gives a clear picture** on this glass. A full frame is approximately 12.3 ms. Dirty rows keep a partial update in the glass period, and a still pose sends nothing.
16. **The bowl does not flip.**
17. If a `full_frame` pile of rigids misses the glass limit, that frame runs at its work time. Do not add a second clock.
18. In the simulator, **a click without a drag is a CST816 tap**. A drag of more than a few pixels is the IMU. Do not give left-drag more functions.
19. Set **`DisAutoSleep`** on the CST816 from the first day. If you do not, the chip ignores the first tap. This is a usual problem.
20. GPIO3 is `CHG_STAT` and an S3 strapping pin. Make a bring-up note if a USB connection at reset causes a conflict with Serial/JTAG.
21. **Do not add a debug LED on GPIO18** (TF D1).
22. Lock the `CASET`/`RASET` offsets before you trust the dirty rect.

---

## 18. Open questions

**Locked in this pass:** live flat room, no photograph. The room sets the camera. `full_frame` only on a clip. Nest, Play, Hall, Yard. One six-bone smooth skin, two influences, the core spring only. Pose mailbox: Core 0 simulates, Core 1 presents. Indexed-8, one framebuffer, two DMA bands, palette of 256 (1–63 actor, 64–255 room). Awake rigids: **24**. FX: **256** stamps. 8 h parked. SPI **mode 3 at 80 MHz** (12271 µs). One I2C owner. I2C addresses 0x6B, 0x15, 0x18. PA gate times. ES7210 **never probed**. No RTC IC. GPIO18 is not a spare pin. CST816 `EnDClick` **exists** (`ChipID` `0xB6`). Light-sleep wake: timer, 48 and 4. One 12 kHz play buffer (300 ms, DRAM). Procedural impacts. s8 chirps. No mixer task. Lizard-brain *when* = later.

1. **PLUS / double-tap:** call the pet, send it to the Nest, or ignore? TBD with the brain. Not volume. Not camera recenter. The hardware can give `GestureID` `0x0B`. The policy is for the lizard brain.
2. **Play and Nest:** two S rooms are locked for v1. Do not invent a fifth room.
3. **Lizard brain:** all of the policy is TBD. Hunger decay, wander, gaze, sleep, think, wave, poke mappings, cortex `action` → clip or room. The eat *when* (the bowl magnet is locked).
4. **Impact decay and chirp takes.** The machinery is locked. The sounds are data. The MX1.25 coil already played a 440 Hz sine.
5. **Last frame on face-down:** Nest sleep pose, or freeze the current room? TBD with the brain.
6. **How strong a `full_frame` shake can be:** the triangle limit and the overdraw are locked. The limit is the glass (12500 µs). A slower frame runs at its work time. There is no second clock.
7. **Desk visit:** only on the host, while USB is connected. Not a fifth room, not a door, not the cortex. Refer to [docs/studio.md](docs/studio.md).

**Already locked (repeated here):** the room is triangles on each frame. The pet is one skinned mesh. World down is gravity. No kitchen. No follow camera in L. No 4-yaw atlas. No backdrop. No z-buffer. The bowl does not flip. This firmware starts Wi-Fi, so that the heap is the same as in the cortex case. The pet is complete when the radio is off. One I2C owner. ES7210 never probed. GPIO18 is not a spare pin.

---

The device is a habitat locked to gravity on this Waveshare board. The rooms are live flat dioramas: a slab, a small number of props, and a sky of hard bands. The creature is one smooth skin on six bones. Only the core has a spring. A floor stamp is below its feet. Toys can move around and then sleep. A clip can shake the camera or move it close. All other frames keep the window of the room. Impacts make a thud, and clips can chirp. The lizard brain does not need a PC. The cortex is a guest.
