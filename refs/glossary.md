# Glossary

Plain-language terms for the [learn course](learn/00-start-here.md). **In ESPET** is how *this* toy uses the word.

Jump: [C and memory](#c-and-memory) · [Buses and hardware](#buses-and-hardware) · [RTOS](#rtos-and-concurrency) · [Graphics](#graphics) · [IMU and math](#imu-and-math) · [Audio](#audio) · [Power](#power) · [Toolchain](#esp-idf-and-toolchain)

Course: [learn/00](learn/00-start-here.md) · [cheat sheet](learn/cheatsheet.md)

---

## C and memory

### Atomic
A read or write the CPU treats as one step, with defined ordering between cores.
**In ESPET:** `_Atomic` on seqlock index and `SfxEvt` ring pointers. See [lesson 03](learn/03-tasks-cores-timing.md).

### Bounce buffer
A small DRAM array you fill just before DMA, instead of pointing DMA at a huge or unreachable buffer.
**In ESPET:** RGB565 scanline (or few rows) when expanding indexed-8. See [lesson 07](learn/07-indexed-framebuffer.md).

### Const
A promise you will not write this object. The compiler may put it in flash.
**In ESPET:** cold tables (`dodeca_vertex`, clip headers) can be `const` for XIP. See [lesson 01](learn/01-c-for-firmware.md).

### DRAM
Internal SRAM the CPU and most DMA can use without the PSRAM cache dance.
**In ESPET:** framebuffers, mixer, seqlock, blit inner loop. See [guide 02](guides/02-soc-memory-smp.md).

### DRAM_ATTR
IDF attribute that forces a symbol into internal DRAM.
**In ESPET:** `g_shared[]`, `g_sfx[]`. See [lesson 03](learn/03-tasks-cores-timing.md).

### Endianness
Byte order of a multi-byte number. Little-endian = least significant byte first.
**In ESPET:** IMU samples and UDP packets are little-endian (`le16`). See [lesson 01](learn/01-c-for-firmware.md).

### Flash
Non-volatile NOR memory the chip boots from. Slow to erase; can execute in place (XIP).
**In ESPET:** 16 MB W25Q128, quad, 80 MHz. Firmware, atlas, clips. No PCM in v1. See [lesson 13](learn/13-power-and-boot.md).

### Heap
Memory `malloc` hands out at runtime. Can fragment.
**In ESPET:** not in the 33 ms hot path. Prefer static/`DRAM_ATTR` buffers. See [lesson 01](learn/01-c-for-firmware.md).

### Hot path
Code that runs every sample or every frame.
**In ESPET:** Core 1 loop and the mixer. No STL, no flash reads for audio, no PSRAM in the blit. See [architecture 0](../architecture.md#0-product-lock).

### int16_t
Exactly 16-bit signed integer (−32768…32767).
**In ESPET:** Q8 rest tracks, IMU raw. See [lesson 01](learn/01-c-for-firmware.md).

### LSB
Least significant bit, or “counts per unit” in a sensor (LSB/g).
**In ESPET:** hello firmware uses 4096 LSB/g; confirm against the QMI8658 scale you set. See [lesson 08](learn/08-imu-registers.md).

### Pointer
An address of another object in memory.
**In ESPET:** `draw_bitmap` takes a pointer to pixels; do not pass a PSRAM atlas into DMA casually. See [lesson 01](learn/01-c-for-firmware.md).

### POD
Plain Old Data: a struct of C types with no hidden constructors.
**In ESPET:** `Bodies`, `ClipHdr`, `SynthPatch`, `SharedSnap`. See [lesson 01](learn/01-c-for-firmware.md).

### PSRAM
Extra RAM outside the CPU die, reached through a cache (here 8 MB octal in-package).
**In ESPET:** optional atlas home if XIP thrashes. Never FB, mixer, or Wi-Fi DMA. See [guide 02](guides/02-soc-memory-smp.md).

### Q8
Fixed-point format: stored_integer / 256 = real value.
**In ESPET:** `emotion_rest` and clip tracks. See [lesson 01](learn/01-c-for-firmware.md).

### RTC SRAM
A little RAM that survives deep sleep.
**In ESPET:** hunger, happy, sleep, last emotion. See [lesson 16](learn/16-sleep-and-battery.md).

### SoA
Struct of arrays: `pos[N]`, `vel[N]` instead of `Body bodies[N]`.
**In ESPET:** `Bodies` is SoA, width 6. See [lesson 01](learn/01-c-for-firmware.md).

### SRAM
On-chip static RAM. On S3, “internal SRAM/DRAM” vs PSRAM.
**In ESPET:** 512 KB on-die; treat it as scarce. See [guide 02](guides/02-soc-memory-smp.md).

### Static
File-scope: private to this `.c`. Local `static`: one copy for the whole program.
**In ESPET:** `s_fb` in `main.c` is file-static. See [lesson 01](learn/01-c-for-firmware.md).

### Struct
A group of named fields laid out in memory.
**In ESPET:** packed packets for UDP; ordinary structs for `ClipHdr`. See [lesson 01](learn/01-c-for-firmware.md).

### uint8_t
Exactly 8-bit unsigned (0…255).
**In ESPET:** palette indices, `clip_id`, GPIO-sized flags. See [lesson 01](learn/01-c-for-firmware.md).

### Volatile
Tells the compiler “this may change behind your back.” Not a CPU memory barrier on SMP Xtensa.
**In ESPET:** do not use `volatile` as the seqlock. Use `_Atomic`. See [lesson 03](learn/03-tasks-cores-timing.md).

### XIP
Execute / read in place from flash via the cache.
**In ESPET:** cold tables. If the atlas thrashes the cache, move pixels to PSRAM. See [guide 02](guides/02-soc-memory-smp.md).

---

## Buses and hardware

### ACK
Acknowledge: the slave pulls SDA low to say “I heard you.”
**In ESPET:** ES8311 bring-up is “I2C ACK on 0x18” before any sine. See [lesson 02](learn/02-how-chips-talk.md).

### ADC
Analog-to-digital converter: voltage → number.
**In ESPET:** GPIO1 `BAT_ADC` through a divider. See [lesson 13](learn/13-power-and-boot.md).

### BCLK
I2S bit clock (one pulse per data bit).
**In ESPET:** GPIO9. See [lesson 15](learn/15-audio.md).

### CLK
SPI clock.
**In ESPET:** GPIO38 to the ST7789. See [lesson 02](learn/02-how-chips-talk.md).

### CS
Chip select: the SPI slave listens only while this line is active (usually low).
**In ESPET:** GPIO21 LCD CS. See [lesson 02](learn/02-how-chips-talk.md).

### CST816
Capacitive touch controller (family S/T/D) on I2C.
**In ESPET:** one finger, INT 48, RST 47, double-tap recenter. See [lesson 14](learn/14-touch.md).

### DC
Data/command pin for SPI LCDs. 0 = command, 1 = parameter or pixels.
**In ESPET:** GPIO45, also a strapping pin. See [lesson 02](learn/02-how-chips-talk.md).

### DMA
Direct Memory Access: a peripheral copies memory without the CPU looping.
**In ESPET:** SPI colour bursts and I2S sample bursts. See [lesson 02](learn/02-how-chips-talk.md).

### ES7210
I2S ADC / mic codec on the same I2C bus.
**In ESPET:** off in v1. Do not init. See [lesson 15](learn/15-audio.md).

### ES8311
Low-power audio codec (DAC + ADC). v1 uses the DAC.
**In ESPET:** I2S TX at 12 kHz, I2C 0x18. See [lesson 15](learn/15-audio.md).

### ESP32-S3
Espressif dual-core Wi-Fi SoC (Xtensa LX7).
**In ESPET:** specifically **S3R8** (8 MB octal PSRAM in package). See [guide 02](guides/02-soc-memory-smp.md).

### FIFO
First-in first-out buffer inside a chip, holding several samples.
**In ESPET:** QMI8658 FIFO drained on watermark INT. See [lesson 08](learn/08-imu-registers.md).

### GDMA
The ESP32-S3 general DMA engine behind SPI/I2S/etc.
**In ESPET:** dirty-rect pixels and mix blocks. See [guide 03](guides/03-display-st7789.md).

### GPIO
General-purpose input/output pin.
**In ESPET:** pin law on the cheat sheet. See [lesson 02](learn/02-how-chips-talk.md).

### I2C
Two-wire serial bus (SCL + SDA) with addressed slaves.
**In ESPET:** one bus GPIO 41/42, one Core 0 owner. See [lesson 02](learn/02-how-chips-talk.md).

### I2C address
7-bit name of a slave on the bus.
**In ESPET:** IMU 0x6A/0x6B, touch 0x15, ES8311 0x18. See [cheatsheet](learn/cheatsheet.md).

### I2S
Serial bus for audio samples (clocks + data, not “files”).
**In ESPET:** TX to ES8311; DIN unused v1. See [lesson 15](learn/15-audio.md).

### IMU
Inertial measurement unit: accelerometer + gyro (here, no magnetometer).
**In ESPET:** QMI8658. See [lesson 08](learn/08-imu-registers.md).

### IRQ
Interrupt request: a pin or peripheral asking the CPU to run an ISR.
**In ESPET:** IMU GPIO6, touch GPIO48. See [lesson 03](learn/03-tasks-cores-timing.md).

### LEDC
ESP32 LED PWM controller. Useful for backlights, not only LEDs.
**In ESPET:** GPIO46 backlight. See [lesson 06](learn/06-first-pixels.md).

### MCLK
Master clock for a codec, faster than the sample rate.
**In ESPET:** GPIO8 to ES8311. Gate it when idle. See [lesson 15](learn/15-audio.md).

### MISO
SPI master-in slave-out. Data back to the CPU.
**In ESPET:** unused on the LCD (write-only). See [lesson 02](learn/02-how-chips-talk.md).

### MMIO
Memory-mapped I/O: peripherals appear as addresses.
**In ESPET:** IDF drivers wrap this. See [lesson 02](learn/02-how-chips-talk.md).

### MOSI
SPI master-out slave-in. Data to the panel.
**In ESPET:** GPIO39. See [lesson 02](learn/02-how-chips-talk.md).

### NS4150B
Filterless Class-D speaker amplifier with a CTRL pin.
**In ESPET:** GPIO7 PA enable. See [lesson 15](learn/15-audio.md).

### Peripheral
On-chip hardware block (SPI, I2C, ADC) or an off-chip IC.
**In ESPET:** keep the list small: SPI3 LCD, one I2C, one I2S TX, ADC1, LEDC, USB Serial/JTAG. See [lesson 02](learn/02-how-chips-talk.md).

### QMI8658
QST 6-axis IMU (accel + gyro).
**In ESPET:** I2C, FIFO, no mag. See [lesson 08](learn/08-imu-registers.md).

### Register
A named byte inside a chip you read or write.
**In ESPET:** you program QMI8658, CST816, ES8311, ST7789 this way. See [lesson 02](learn/02-how-chips-talk.md).

### RST
Reset pin. Pulse it and the chip starts over.
**In ESPET:** LCD 40, touch 47. See [cheatsheet](learn/cheatsheet.md).

### SA0
Address-select pin on some I2C chips.
**In ESPET:** QMI8658 0x6A vs 0x6B. Hello uses 0x6B. See [lesson 08](learn/08-imu-registers.md).

### SCL
I2C clock.
**In ESPET:** GPIO41. See [lesson 02](learn/02-how-chips-talk.md).

### SDA
I2C data.
**In ESPET:** GPIO42. See [lesson 02](learn/02-how-chips-talk.md).

### Schematic
The wiring diagram of the board.
**In ESPET:** law after one pass. [`ESP32-S3-LCD-1.54-Schematic.pdf`](ESP32-S3-LCD-1.54-Schematic.pdf). See [lesson 02](learn/02-how-chips-talk.md).

### SoC
System on chip: CPU + RAM + radios + peripherals in one package.
**In ESPET:** ESP32-S3R8. See [guide 02](guides/02-soc-memory-smp.md).

### SPI
Clocked serial bus, usually one slave per CS, much faster than I2C.
**In ESPET:** ST7789 on SPI3. See [lesson 02](learn/02-how-chips-talk.md).

### SPI mode
Combination of clock polarity and phase (0…3).
**In ESPET:** try 0, confirm vs 3 on glass. See [lesson 06](learn/06-first-pixels.md).

### ST7789
Sitronix LCD controller with on-panel GRAM. This module is the V2 variant, 240×240 window on 240×320 RAM.
**In ESPET:** 4-wire SPI, RGB565, no TE. See [lesson 06](learn/06-first-pixels.md).

### Strapping pin
A GPIO sampled *at reset* to choose boot mode or voltages.
**In ESPET:** 45 = DC, 46 = backlight; 0 = BOOT. See [lesson 13](learn/13-power-and-boot.md).

### TE
Tearing-effect pin: the panel pulses at vblank.
**In ESPET:** **no TE**. 30 FPS is software. See [lesson 06](learn/06-first-pixels.md).

### TRM
Technical Reference Manual: how the SoC peripherals really work.
**In ESPET:** [`esp32-s3_technical_reference_manual_en.pdf`](soc/esp32-s3_technical_reference_manual_en.pdf). See [guide 02](guides/02-soc-memory-smp.md).

### UART
Old-school serial port (TX/RX bytes).
**In ESPET:** you use USB CDC instead of a USB-UART bridge. Pads exist for debug. See [lesson 13](learn/13-power-and-boot.md).

### USB CDC
USB as a virtual COM port.
**In ESPET:** native USB Serial/JTAG on 19/20. See [lesson 13](learn/13-power-and-boot.md).

### Watermark
FIFO level that fires an interrupt.
**In ESPET:** QMI8658 `FIFO_WTM_TH` → GPIO6. See [lesson 08](learn/08-imu-registers.md).

### WHO_AM_I
ID register. If it is not the expected value, you have the wrong address or a dead chip.
**In ESPET:** QMI8658 expects `0x05` on this board/sim. See [lesson 08](learn/08-imu-registers.md).

### WS
I2S word select (LRCK): marks left/right or frame start.
**In ESPET:** GPIO10. See [lesson 15](learn/15-audio.md).

---

## RTOS and concurrency

### Blocking
A call that waits until something happens (`vTaskDelay`, I2C complete).
**In ESPET:** Core 1 must not block on Core 0, Wi-Fi, or I2S. See [lesson 03](learn/03-tasks-cores-timing.md).

### CCOUNT
Xtensa cycle counter. Cheap high-resolution clock.
**In ESPET:** lock the 33.3 ms frame. See [lesson 03](learn/03-tasks-cores-timing.md).

### Core
One CPU of the dual LX7.
**In ESPET:** Core 0 sensors/mixer; Core 1 body. See [lesson 03](learn/03-tasks-cores-timing.md).

### FreeRTOS
Tiny real-time OS IDF is built on (tasks, queues, delays).
**In ESPET:** `CONFIG_FREERTOS_HZ=1000`. See [lesson 03](learn/03-tasks-cores-timing.md).

### GPTimer
Hardware timer peripheral.
**In ESPET:** alternative to CCOUNT for the frame lock. See [lesson 03](learn/03-tasks-cores-timing.md).

### ISR
Interrupt service routine: tiny function run because hardware yelled.
**In ESPET:** no I2C inside. See [lesson 03](learn/03-tasks-cores-timing.md).

### Pinning
Forcing a task to always run on one core.
**In ESPET:** the body loop is pinned to Core 1. See [lesson 03](learn/03-tasks-cores-timing.md).

### Priority
RTOS ranking. Higher runs first when both are ready.
**In ESPET:** IMU 12, touch 11, mixer 7, house 5. See [architecture 4](../architecture.md#4-core-allocation).

### Seqlock
A sequence-number lock: writer bumps a counter; reader retries if it changed mid-copy.
**In ESPET:** `SharedSnap g_shared[2]`. See [lesson 03](learn/03-tasks-cores-timing.md).

### SMP
Symmetric multiprocessing: two CPUs, one memory.
**In ESPET:** why `volatile` is not enough. See [lesson 03](learn/03-tasks-cores-timing.md).

### SPSC
Single-producer single-consumer queue.
**In ESPET:** `SfxEvt` ring, overwrite-oldest. See [lesson 12](learn/12-clips-springs-hitboxes.md).

### Task
A function that looks like it has its own `for(;;)` and stack, scheduled by FreeRTOS.
**In ESPET:** `app_main` is already one. See [lesson 03](learn/03-tasks-cores-timing.md).

### Tick
RTOS time unit. At 1000 Hz, one tick = 1 ms.
**In ESPET:** `pdMS_TO_TICKS(33)`. See [lesson 03](learn/03-tasks-cores-timing.md).

### WFI
Wait for interrupt: the CPU sleeps until the next event.
**In ESPET:** Core 1 slack each frame. See [lesson 16](learn/16-sleep-and-battery.md).

### Xtensa
The CPU architecture (LX7 here).
**In ESPET:** CCOUNT, windowed registers, SMP atomics. See [lesson 03](learn/03-tasks-cores-timing.md).

---

## Graphics

### AABB
Axis-aligned bounding box (min/max x,y, no rotation).
**In ESPET:** dirty rect = union of projected part AABBs + margin. See [lesson 07](learn/07-indexed-framebuffer.md).

### Atlas
One (or few) big images packing many sprites.
**In ESPET:** indexed-8, same 32-colour palette as the FB. See [lesson 11](learn/11-sheets-and-sprites.md).

### Billboard
A sprite quad that faces the camera.
**In ESPET:** face the *live* boom, not the bake camera. See [lesson 11](learn/11-sheets-and-sprites.md).

### Blit
Copy (and maybe key) pixels onto the framebuffer.
**In ESPET:** five part sprites after the room. See [lesson 11](learn/11-sheets-and-sprites.md).

### CASET
ST7789 “column address set” — window X range before RAMWR.
**In ESPET:** dirty rect X. See [lesson 07](learn/07-indexed-framebuffer.md).

### Color key
A reserved index meaning “transparent.”
**In ESPET:** index 0. See [lesson 07](learn/07-indexed-framebuffer.md).

### COLMOD
ST7789 colour-mode command. `0x55` = RGB565.
**In ESPET:** 16 bpp on the wire. See [lesson 06](learn/06-first-pixels.md).

### Dirty rect
The smallest rectangle that changed and must be sent to the panel.
**In ESPET:** why 30 FPS is possible at 40 MHz SPI. See [lesson 07](learn/07-indexed-framebuffer.md).

### Dodecahedron
A 12-face solid whose 20 vertices are evenly spaced directions.
**In ESPET:** texture index only, ~37° apart. Vertex 0 = +Y. See [lesson 11](learn/11-sheets-and-sprites.md).

### FOV
Field of view of the perspective camera.
**In ESPET:** ~55°, same in Blender as runtime. See [lesson 04](learn/04-vectors-matrices-camera.md).

### FPS
Frames per second.
**In ESPET:** cap 30. Physics in the same 33 ms loop. See [lesson 03](learn/03-tasks-cores-timing.md).

### GRAM
The LCD controller’s own pixel RAM.
**In ESPET:** holds the last frame when you skip SPI. See [lesson 07](learn/07-indexed-framebuffer.md).

### Hotspot
Pixel in a sprite that sits on the 3D attach point (shoulder, hip).
**In ESPET:** `hot_x, hot_y` in `SpriteRec`. See [lesson 11](learn/11-sheets-and-sprites.md).

### Impostor
Using a photo of a 3D object instead of triangles.
**In ESPET:** part sheets. Mismatch vs live camera is accepted. See [lesson 11](learn/11-sheets-and-sprites.md).

### Indexed-8
Each pixel is a 1-byte palette index, not an RGB colour.
**In ESPET:** FB and atlas. Expand only on scanout. See [lesson 07](learn/07-indexed-framebuffer.md).

### MADCTL
ST7789 memory-access-control: axis flip and RGB vs BGR.
**In ESPET:** set once on bring-up; touch axes must match. See [lesson 06](learn/06-first-pixels.md).

### Palette
Table mapping index → RGB565.
**In ESPET:** 32 colours. See [lesson 07](learn/07-indexed-framebuffer.md).

### Perspective divide
After projection, `xyz / w` to get normalized device coordinates.
**In ESPET:** inside `project(pos)`. See [lesson 04](learn/04-vectors-matrices-camera.md).

### Projection matrix
Turns camera-space points into clip space (FOV, near, far).
**In ESPET:** square aspect 1.0. See [lesson 04](learn/04-vectors-matrices-camera.md).

### RASET
ST7789 row address set — window Y range.
**In ESPET:** dirty rect Y. See [lesson 07](learn/07-indexed-framebuffer.md).

### RAMWR
ST7789 “write to GRAM” — the pixel burst.
**In ESPET:** DMA this. See [lesson 06](learn/06-first-pixels.md).

### RGB565
16-bit colour: 5 red, 6 green, 5 blue.
**In ESPET:** only on the SPI wire and bounce buffer. See [lesson 06](learn/06-first-pixels.md).

### Scanout
Turning the CPU’s framebuffer into panel pixels.
**In ESPET:** indexed → RGB565 bounce → GDMA. See [lesson 07](learn/07-indexed-framebuffer.md).

### Sheet
One baked photo of a part from one dodecahedron camera.
**In ESPET:** appearance, not physics. See [lesson 11](learn/11-sheets-and-sprites.md).

### SpriteRec
Record of where a sprite lives in the atlas (offset, size, hotspot).
**In ESPET:** architecture §10. See [lesson 11](learn/11-sheets-and-sprites.md).

### View matrix
World → camera transform from `look_at`.
**In ESPET:** live IMU boom, never a bake vertex. See [lesson 10](learn/10-gravity-locked-cube.md).

### view_idx
Integer 0…19: which sheet to blit.
**In ESPET:** discrete; the camera is not. Hysteresis ~0.02. See [lesson 11](learn/11-sheets-and-sprites.md).

---

## IMU and math

### Accelerometer
Sensor for specific force. At rest, it reads gravity.
**In ESPET:** defines down (pitch/roll). See [lesson 05](learn/05-quaternions.md).

### Bias
A slowly wrong offset, especially on gyro.
**In ESPET:** optional `ki` integrator in the filter; freeze when `|a|` is not ~1 g. See [lesson 09](learn/09-complementary-filter.md).

### Body space
Pet-local coordinates: core at origin, yaw 0.
**In ESPET:** springs live here. See [lesson 12](learn/12-clips-springs-hitboxes.md).

### Boom camera
A camera on a stick looking at a focus point.
**In ESPET:** focus = pet core, length chosen so the pet is ~1/3 of the frame. See [lesson 10](learn/10-gravity-locked-cube.md).

### Clip
A short animation: keyframed rest poses (and matching sheets), not a film of the whole pet.
**In ESPET:** writes `rest[]`, optional `vox_id` on start. Policy of *when* to play is TBD. See [lesson 12](learn/12-clips-springs-hitboxes.md).

### Complementary filter
Fuse sensors that are good in different frequency bands (gyro high-freq, accel low-freq).
**In ESPET:** 100 Hz `q_device_to_world`. See [lesson 09](learn/09-complementary-filter.md).

### Deadband
Ignore changes smaller than a threshold so noise does not keep the system “busy.”
**In ESPET:** skip SPI unless `|Δq|` / `|Δcam|` beat epsilon. See [lesson 10](learn/10-gravity-locked-cube.md).

### Cross product
Vector perpendicular to two inputs.
**In ESPET:** IMU error `a × predicted_down`; `look_at` axes. See [lesson 04](learn/04-vectors-matrices-camera.md).

### Dot product
`a·b`; measures alignment.
**In ESPET:** `view_idx = argmax(dot(dir, dodeca[i]))`. See [lesson 04](learn/04-vectors-matrices-camera.md).

### Euler angles
Yaw, pitch, roll as three angles.
**In ESPET:** debug overlay only, not the state. See [lesson 05](learn/05-quaternions.md).

### Gimbal lock
Losing a degree of freedom with Euler angles at some poses.
**In ESPET:** why quaternions. See [lesson 05](learn/05-quaternions.md).

### Gyro
Gyroscope: angular velocity.
**In ESPET:** smooths tilt; only source of yaw. See [lesson 05](learn/05-quaternions.md).

### Hysteresis
Do not switch until the new choice beats the old by a margin.
**In ESPET:** sheets only, ~0.02 on the dot. See [lesson 11](learn/11-sheets-and-sprites.md).

### Hitbox
Simple shape used for collision, not the visible pixels.
**In ESPET:** spheres on `pos` (and optional tips). See [lesson 12](learn/12-clips-springs-hitboxes.md).

### Jerk
High-pass / sudden change of acceleration.
**In ESPET:** flinch, not camera. See [lesson 09](learn/09-complementary-filter.md).

### look_at
Build a view matrix from eye, target, up.
**In ESPET:** architecture §1. See [lesson 04](learn/04-vectors-matrices-camera.md).

### Magnetometer
Compass sensor.
**In ESPET:** **none**. Yaw drifts. See [lesson 05](learn/05-quaternions.md).

### Normalize
Scale a vector or quaternion to length 1.
**In ESPET:** after every quat integrate. See [lesson 05](learn/05-quaternions.md).

### Observability
What a sensor set can actually determine.
**In ESPET:** pitch/roll yes; yaw around gravity no. See [lesson 05](learn/05-quaternions.md).

### Quaternion
Four-number rotation (`x,y,z,w` in `SharedSnap`).
**In ESPET:** `q_device_to_world`. See [lesson 05](learn/05-quaternions.md).

### Recenter
Reset drifted yaw to a “front.”
**In ESPET:** PLUS and CST816 double-tap. See [lesson 09](learn/09-complementary-filter.md).

### Rotation matrix
3×3 (or the upper 3×3 of a 4×4) that rotates vectors.
**In ESPET:** `inv_R_core` for `dir_local`. See [lesson 04](learn/04-vectors-matrices-camera.md).

### Vector
`(x,y,z)` point or direction.
**In ESPET:** rest, pos, cam, gravity. See [lesson 04](learn/04-vectors-matrices-camera.md).

### Verlet
A way to step positions using previous position (or velocity) that is stable for simple physics.
**In ESPET:** core mass vs floor, plus springs on limbs. See [lesson 12](learn/12-clips-springs-hitboxes.md).

### World space
Coordinates glued to the Earth. `+Y` up.
**In ESPET:** room and gravity. Pet does not lean with the glass. See [lesson 10](learn/10-gravity-locked-cube.md).

### Yaw
Rotation around world up.
**In ESPET:** gyro-only, recenter required. See [lesson 05](learn/05-quaternions.md).

---

## Audio

### AEC
Acoustic echo cancellation (mic hears the speaker).
**In ESPET:** not v1. Do not steal XiaoZhi’s duplex graph. See [lesson 15](learn/15-audio.md).

### Codec
Chip that turns I2S numbers into analog (or the reverse).
**In ESPET:** ES8311 DAC. See [lesson 15](learn/15-audio.md).

### Mixer
Sums voices into one sample stream.
**In ESPET:** Core 0, two voices, 12 kHz, block 256. See [lesson 15](learn/15-audio.md).

### PA
Power amplifier for the speaker.
**In ESPET:** NS4150B, GPIO7 high only while a voice is live. See [lesson 15](learn/15-audio.md).

### PCM
Stored waveform samples.
**In ESPET:** none in v1. Procedural patches. Pak has a reserved PCM appendix. See [lesson 15](learn/15-audio.md).

### Procedural audio
Sound generated by oscillators/noise, not a wav file.
**In ESPET:** `SynthPatch` 16 bytes. See [lesson 15](learn/15-audio.md).

### SfxEvt
Tiny event: patch id, velocity, tag.
**In ESPET:** Core 1 → Core 0 ring. See [lesson 12](learn/12-clips-springs-hitboxes.md).

### SynthPatch
POD describing one sound.
**In ESPET:** flash → DRAM at boot. See [lesson 15](learn/15-audio.md).

### Tail
The decay after a note, before silence.
**In ESPET:** finish tail before PA low / light-sleep (~800 ms cap). See [lesson 16](learn/16-sleep-and-battery.md).

### Voice
One mixer slot. Last event of that class wins.
**In ESPET:** A = impact, B = creature. See [lesson 15](learn/15-audio.md).

### vox_id
Patch to play when a clip **starts**. 0 = silent.
**In ESPET:** on `ClipHdr`, not a parallel lizard trigger. See [lesson 12](learn/12-clips-springs-hitboxes.md).

---

## Power

### BAT_EN
Board net that keeps battery power latched on.
**In ESPET:** GPIO2, hold high or the board dies. See [lesson 13](learn/13-power-and-boot.md).

### CHG_STAT
Charger status pin.
**In ESPET:** GPIO3, ETA6098 STAT. See [lesson 13](learn/13-power-and-boot.md).

### Deep sleep
CPU off; only RTC and configured pads live.
**In ESPET:** after tail, PA low, `BAT_EN` held. See [lesson 16](learn/16-sleep-and-battery.md).

### ETA6098
Switching Li-ion charger IC. No I2C map.
**In ESPET:** hardware only; you read STAT and VBAT. See [lesson 13](learn/13-power-and-boot.md).

### Light sleep
CPU paused, peripherals gated; faster wake than deep sleep.
**In ESPET:** only if mixer idle. See [lesson 16](learn/16-sleep-and-battery.md).

### PMIC
Power-management IC with I2C (e.g. AXP).
**In ESPET:** **none**. See [lesson 13](learn/13-power-and-boot.md).

### PWR
Power button.
**In ESPET:** GPIO5, long-press latch off. See [lesson 13](learn/13-power-and-boot.md).

### Studio mode
USB plugged in: bright, 30 FPS, no light-sleep, cortex allowed.
**In ESPET:** architecture §14. See [lesson 16](learn/16-sleep-and-battery.md).

### VBAT
Battery voltage.
**In ESPET:** ADC GPIO1 via divider. See [lesson 13](learn/13-power-and-boot.md).

---

## ESP-IDF and toolchain

### app_main
Your entry after IDF init. Already a FreeRTOS task.
**In ESPET:** [`firmware/main.c`](../firmware/main.c). See [lesson 06](learn/06-first-pixels.md).

### Arduino
Hobby framework on top of similar chips.
**In ESPET:** not used. See [architecture 0](../architecture.md#0-product-lock).

### board-sim
Windows fake Waveshare: GRAM window + fake IMU. No cube logic.
**In ESPET:** `sim.bat`. See [lesson 00](learn/00-start-here.md).

### Bring-up
First-time hardware tests in a fixed order.
**In ESPET:** architecture §15 and [guide 09](guides/09-bring-up.md). See [lesson 13](learn/13-power-and-boot.md).

### Cortex
Optional LAN brain (UDP to a PC LLM). Guest, not required.
**In ESPET:** radio off by default. Behaviour TBD. See [architecture 13](../architecture.md#13-optional-cortex-wi-fi-is-a-mode).

### esp_lcd
IDF component for panels (SPI IO + ST7789 helper).
**In ESPET:** init + `draw_bitmap`; you own windows. See [lesson 06](learn/06-first-pixels.md).

### ESP-IDF
Espressif’s official SDK (CMake, compilers, drivers).
**In ESPET:** ≥ 5.5, C++ as better C. See [lesson 13](learn/13-power-and-boot.md).

### idf.py
IDF command-line: `build`, `flash`, `monitor`.
**In ESPET:** real board only, not `sim.bat`. See [lesson 13](learn/13-power-and-boot.md).

### Lizard brain
On-device behaviour (wander, wave, sleep) with radio off.
**In ESPET:** **policy TBD**. Machinery exists. Do not invent a personality. See [lesson 00](learn/00-start-here.md).

### LVGL
Popular GUI library.
**In ESPET:** forbidden. See [architecture 0](../architecture.md#0-product-lock).

### sdkconfig
Kconfig output that compiles into IDF.
**In ESPET:** octal PSRAM 80 M, quad flash 80 M, tick 1000, BT off. See [cheatsheet](learn/cheatsheet.md).

### XiaoZhi
Voice-assistant firmware some Waveshare demos ship.
**In ESPET:** steal **pins** only, not 24 kHz duplex/AEC. See [lesson 15](learn/15-audio.md).
