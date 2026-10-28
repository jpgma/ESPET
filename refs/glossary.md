# Glossary

This glossary gives simple definitions for the terms of the [learn course](./learn/00-start-here.md). **In ESPET** tells how *this* toy uses the term.

For the board pins, chips and datasheets, refer to **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154/docs/glossary.md`](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/glossary.md). Lessons 02, 06, 08 and 13–16 in this folder are stubs that point to that repository.

Go to: [C and memory](#c-and-memory) · [Buses and hardware](#buses-and-hardware) · [RTOS](#rtos-and-concurrency) · [Graphics](#graphics) · [IMU and math](#imu-and-math) · [Audio](#audio) · [Power](#power) · [Toolchain](#esp-idf-and-toolchain)

Course: [learn/00](./learn/00-start-here.md) · [cheat sheet](./learn/cheatsheet.md)

---

## C and memory

### Atomic
A read or a write that the CPU does as one step, with a defined order between the cores.
**In ESPET:** `_Atomic` on the seqlock index. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### Bounce buffer
A small DRAM array that you fill immediately before DMA. You use it instead of a very large buffer, or a buffer that DMA cannot get to.
**In ESPET:** an RGB565 scanline (or a small number of rows) during the expansion of indexed-8. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### Const
A promise that you will not write this object. The compiler can put it in flash.
**In ESPET:** cold tables (4 yaw headings for the draw order, clip headers, room records, meshes) can be `const` for XIP. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### DRAM
Internal SRAM. The CPU and most DMA can use it without the PSRAM cache.
**In ESPET:** framebuffers, play buffer, seqlock, raster and blit inner loop. Refer to [guide 02](./guides/02-soc-memory-smp.md).

### DRAM_ATTR
An IDF attribute that puts a symbol in internal DRAM.
**In ESPET:** `g_shared[]` and the play buffer. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### Endianness
The byte order of a multi-byte number. Little-endian = the least significant byte comes first.
**In ESPET:** the IMU samples and the UDP packets are little-endian (`le16`). Refer to [lesson 01](./learn/01-c-for-firmware.md).

### Flash
Non-volatile NOR memory. The chip boots from it. Erase is slow. Code can execute in place (XIP).
**In ESPET:** 16 MB quad NOR, 80 MHz. The JEDEC ID of this unit is `0x204018` (XMC). The schematic text says W25Q128JVSIQ. Contents: firmware, meshes, stamps, bone clips, creature chirps (s8). Refer to [lesson 13](./learn/13-power-and-boot.md).

### Heap
The memory that `malloc` gives at runtime. It can become fragmented.
**In ESPET:** not in the present hot path. Use static or `DRAM_ATTR` buffers. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### Hot path
Code that runs on each sample or each frame.
**In ESPET:** the present loop on Core 1. No STL. The raster inner loop stays in DRAM. A one-shot can read flash while the amplifier wakes. Then DMA plays the DRAM buffer. Refer to [architecture 0](../architecture.md#0-product-lock).

### int16_t
A signed integer of exactly 16 bits (−32768…32767).
**In ESPET:** Q8 rest tracks, raw IMU data. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### LSB
The least significant bit. In a sensor, it is also the "counts per unit" (LSB/g).
**In ESPET:** start with **±8 g → 4096 LSB/g** until CTRL2 changes. Refer to [lesson 08](./learn/08-imu-registers.md).

### Pointer
The address of a different object in memory.
**In ESPET:** `draw_bitmap` takes a pointer to pixels. Do not give a PSRAM buffer to DMA without a good reason. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### POD
Plain Old Data: a struct of C types with no hidden constructors.
**In ESPET:** `Bodies`, `ClipHdr`, `ImpactPatch`, `ChirpHdr`, `SharedSnap`. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### PSRAM
More RAM outside the CPU die. The CPU gets to it through a cache. (Here, 8 MB octal in the package.)
**In ESPET:** only optional cold copies. Never the framebuffer, the pose, the play buffer or Wi-Fi DMA. Meshes stay in XIP. No backdrop. Refer to [guide 02](./guides/02-soc-memory-smp.md).

### Q8
A fixed-point format: stored_integer / 256 = real value.
**In ESPET:** `emotion_rest` and clip tracks. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### RTC SRAM
A small RAM that keeps its data in deep sleep.
**In ESPET:** hunger, happy, sleep, last emotion, `room_id`. Refer to [lesson 16](./learn/16-sleep-and-battery.md).

### SoA
Struct of arrays: `pos[N]`, `vel[N]` instead of `Body bodies[N]`.
**In ESPET:** `Bodies` is SoA, with a width of 6. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### SRAM
Static RAM on the chip. On the S3, "internal SRAM/DRAM" is different from PSRAM.
**In ESPET:** 512 KB on the die. There is not much of it. Refer to [guide 02](./guides/02-soc-memory-smp.md).

### Static
At file scope: private to this `.c` file. A local `static`: one copy for the full program.
**In ESPET:** `s_fb` in `main.c` is static at file scope. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### Struct
A group of named fields in memory.
**In ESPET:** packed packets for UDP; usual structs for `ClipHdr`. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### uint8_t
An unsigned integer of exactly 8 bits (0…255).
**In ESPET:** palette indices, `clip_id`, flags of GPIO size. Refer to [lesson 01](./learn/01-c-for-firmware.md).

### Volatile
It tells the compiler: "this value can change without your knowledge". On SMP Xtensa, it is not a CPU memory barrier.
**In ESPET:** do not use `volatile` as the seqlock. Use `_Atomic`. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### XIP
Execute or read in place from flash, through the cache.
**In ESPET:** cold tables and meshes. If the stamps thrash the cache, move those pixels to PSRAM. Refer to [guide 02](./guides/02-soc-memory-smp.md).

---

## Buses and hardware

### ACK
Acknowledge: the slave pulls SDA low to say "I received your message".
**In ESPET:** for the ES8311 bring-up, get an "I2C ACK on 0x18" before you play a sine. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### ADC
Analog-to-digital converter: it changes a voltage into a number.
**In ESPET:** GPIO1 `BAT_ADC` through a divider. Refer to [lesson 13](./learn/13-power-and-boot.md).

### BCLK
The I2S bit clock (one pulse for each data bit).
**In ESPET:** GPIO9. Refer to [lesson 15](./learn/15-audio.md).

### CLK
The SPI clock.
**In ESPET:** GPIO38 to the ST7789. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### CS
Chip select: the SPI slave listens only while this line is active (usually low).
**In ESPET:** GPIO21, LCD CS. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### CST816
A capacitive touch controller (S, T and D versions) on I2C.
**In ESPET:** CST816D (`ChipID` `0xB6`), I2C 0x15, one finger, INT 48, RST 47. `EnDClick` exists. The lizard brain will set the mapping (TBD). In the simulator, a click is a tap and a drag is the IMU. Refer to [lesson 14](./learn/14-touch.md).

### DC
The data/command pin of SPI LCDs. 0 = command, 1 = parameter or pixels.
**In ESPET:** GPIO45. It is also a strapping pin. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### DMA
Direct Memory Access: a peripheral copies memory without a CPU loop.
**In ESPET:** SPI color bursts and I2S sample bursts. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### ES7210
An I2S ADC (microphone codec) on the same I2C bus.
**In ESPET:** off in v1. Do not initialize it. Refer to [lesson 15](./learn/15-audio.md).

### ES8311
A low-power audio codec (DAC and ADC). v1 uses the DAC.
**In ESPET:** I2S TX at 12 kHz, I2C 0x18. Refer to [lesson 15](./learn/15-audio.md).

### ESP32-S3
The Espressif dual-core Wi-Fi SoC (Xtensa LX7).
**In ESPET:** the **S3R8** version (8 MB octal PSRAM in the package). Refer to [guide 02](./guides/02-soc-memory-smp.md).

### FIFO
A first-in, first-out buffer in a chip. It holds several samples.
**In ESPET:** the QMI8658 FIFO. The firmware drains it on the watermark INT. Refer to [lesson 08](./learn/08-imu-registers.md).

### GDMA
The general DMA engine of the ESP32-S3. SPI, I2S and other peripherals use it.
**In ESPET:** dirty-rect pixels and mix blocks. Refer to [guide 03](./guides/03-display-st7789.md).

### GPIO
A general-purpose input/output pin.
**In ESPET:** the pin rules are on the cheat sheet. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### I2C
A two-wire serial bus (SCL and SDA) with addressed slaves.
**In ESPET:** one bus on GPIO 41/42, with one owner on Core 0. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### I2C address
The 7-bit name of a slave on the bus.
**In ESPET:** QMI8658A 0x6B, CST816D 0x15, ES8311 0x18. ES7210 0x40 is present, but it stays not initialized. Refer to the [cheatsheet](./learn/cheatsheet.md).

### I2S
A serial bus for audio samples (clocks and data, not "files").
**In ESPET:** TX to the ES8311. DIN is not used in v1. Refer to [lesson 15](./learn/15-audio.md).

### IMU
Inertial measurement unit: an accelerometer and a gyro (here, no magnetometer).
**In ESPET:** QMI8658. Refer to [lesson 08](./learn/08-imu-registers.md).

### IRQ
Interrupt request: a pin or a peripheral asks the CPU to run an ISR.
**In ESPET:** IMU on GPIO6, touch on GPIO48. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### LEDC
The LED PWM controller of the ESP32. It is also useful for backlights.
**In ESPET:** GPIO46 backlight. Refer to [lesson 06](./learn/06-first-pixels.md).

### MCLK
The master clock of a codec. It is faster than the sample rate.
**In ESPET:** GPIO8 to the ES8311. Stop it when it is idle. Refer to [lesson 15](./learn/15-audio.md).

### MISO
SPI master-in slave-out. Data back to the CPU.
**In ESPET:** not used on the LCD (write only). Refer to [lesson 02](./learn/02-how-chips-talk.md).

### MMIO
Memory-mapped I/O: the peripherals show as addresses.
**In ESPET:** the IDF drivers wrap this. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### MOSI
SPI master-out slave-in. Data to the panel.
**In ESPET:** GPIO39. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### NS4150B
A filterless Class-D speaker amplifier with a CTRL pin.
**In ESPET:** GPIO7 enables the PA. Refer to [lesson 15](./learn/15-audio.md).

### Peripheral
A hardware block on the chip (SPI, I2C, ADC), or an IC outside the chip.
**In ESPET:** keep the list small: SPI3 LCD, one I2C, one I2S TX, ADC1, LEDC, USB Serial/JTAG. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### QMI8658
A QST 6-axis IMU (accelerometer and gyro).
**In ESPET:** I2C, FIFO, no magnetometer. Refer to [lesson 08](./learn/08-imu-registers.md).

### Register
A named byte in a chip. You read it or write it.
**In ESPET:** you program the QMI8658, CST816, ES8311 and ST7789 this way. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### RST
The reset pin. When you pulse it, the chip starts again.
**In ESPET:** LCD 40, touch 47. Refer to the [cheatsheet](./learn/cheatsheet.md).

### SA0
An address-select pin on some I2C chips.
**In ESPET:** this QMI8658A answers only at **0x6B**. `0x6A` does not send ACK. Refer to [lesson 08](./learn/08-imu-registers.md).

### SCL
The I2C clock.
**In ESPET:** GPIO41. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### SDA
The I2C data.
**In ESPET:** GPIO42. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### Schematic
The wiring diagram of the board.
**In ESPET:** after one check, it is the rule. The schematic is in [jpgma/esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/refs/ESP32-S3-LCD-1.54-Schematic.pdf). Refer to [lesson 02](./learn/02-how-chips-talk.md).

### SoC
System on chip: CPU, RAM, radios and peripherals in one package.
**In ESPET:** ESP32-S3R8. Refer to [guide 02](./guides/02-soc-memory-smp.md).

### SPI
A clocked serial bus, usually with one slave for each CS. It is much faster than I2C.
**In ESPET:** ST7789 on SPI3. Refer to [lesson 02](./learn/02-how-chips-talk.md).

### SPI mode
A combination of clock polarity and phase (0…3).
**In ESPET:** mode **3** at 80 MHz gives a clear picture on this glass. Refer to [lesson 06](./learn/06-first-pixels.md).

### ST7789
A Sitronix LCD controller with GRAM on the panel. This module has the V2 version: a 240×240 window on a 240×320 RAM.
**In ESPET:** 4-wire SPI, RGB565, no TE. Refer to [lesson 06](./learn/06-first-pixels.md).

### Strapping pin
A GPIO that the chip reads *at reset* to select the boot mode or the voltages.
**In ESPET:** 45 = DC, 46 = backlight; 0 = BOOT. Refer to [lesson 13](./learn/13-power-and-boot.md).

### TE
Tearing-effect pin: the panel sends a pulse at vblank.
**In ESPET:** **no TE**. The present is in software. The limit is the glass, 80 Hz (12500 µs), not 30 FPS. Refer to [lesson 06](./learn/06-first-pixels.md).

### TRM
Technical Reference Manual: it tells how the SoC peripherals really operate.
**In ESPET:** [`esp32-s3_technical_reference_manual_en.pdf`](https://github.com/jpgma/esp32-s3/blob/main/refs/soc/esp32-s3_technical_reference_manual_en.pdf) in **jpgma/esp32-s3**. Refer to [guide 02](./guides/02-soc-memory-smp.md).

### UART
An old type of serial port (TX/RX bytes).
**In ESPET:** you use USB CDC instead of a USB-UART bridge. Pads are available for debug. Refer to [lesson 13](./learn/13-power-and-boot.md).

### USB CDC
USB as a virtual COM port.
**In ESPET:** native USB Serial/JTAG on 19/20. Refer to [lesson 13](./learn/13-power-and-boot.md).

### Watermark
A FIFO level that causes an interrupt.
**In ESPET:** QMI8658 `FIFO_WTM_TH` → GPIO6. Refer to [lesson 08](./learn/08-imu-registers.md).

### WHO_AM_I
An ID register. If it does not have the expected value, the address is wrong or the chip is defective.
**In ESPET:** on this board and in the simulator, the QMI8658 must give `0x05`. Refer to [lesson 08](./learn/08-imu-registers.md).

### WS
I2S word select (LRCK): it marks left/right or the frame start.
**In ESPET:** GPIO10. Refer to [lesson 15](./learn/15-audio.md).

---

## RTOS and concurrency

### Blocking
A call that waits until an event occurs (`vTaskDelay`, I2C complete).
**In ESPET:** Core 1 must not block for Core 0, Wi-Fi or I2S. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### CCOUNT
The Xtensa cycle counter. A cheap, high-resolution clock.
**In ESPET:** measure the present against the glass limit of 12500 µs. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### Core
One CPU of the dual LX7.
**In ESPET:** Core 0 for the sensors and the sim; Core 1 for the body. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### FreeRTOS
The very small real-time OS that IDF uses (tasks, queues, delays).
**In ESPET:** `CONFIG_FREERTOS_HZ=1000`. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### GPTimer
A hardware timer peripheral.
**In ESPET:** an alternative to CCOUNT for the frame lock. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### ISR
Interrupt service routine: a very small function that runs because of a hardware signal.
**In ESPET:** no I2C in it. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### Pinning
To make a task always run on one core.
**In ESPET:** the body loop is pinned to Core 1. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### Priority
The RTOS rank. When two tasks are ready, the higher one runs first.
**In ESPET:** I2C owner 13, IMU filter 12, sim 6, housekeeping 5. Refer to [architecture 4](../architecture.md#4-core-allocation).

### Seqlock
A sequence-number lock: the writer increments a counter. If the counter changed during the copy, the reader tries again.
**In ESPET:** `SharedSnap g_shared[2]`. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### SMP
Symmetric multiprocessing: two CPUs, one memory.
**In ESPET:** this is why `volatile` is not sufficient. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### SPSC
A single-producer, single-consumer queue.
**In ESPET:** sound does not use a queue. The sim writes the play buffer on the tick. Refer to [lesson 15](./learn/15-audio.md).

### Task
A function with its own `for(;;)` and stack. FreeRTOS schedules it.
**In ESPET:** `app_main` is already a task. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### Tick
The RTOS time unit. At 1000 Hz, one tick = 1 ms.
**In ESPET:** `pdMS_TO_TICKS(33)`. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### WFI
Wait for interrupt: the CPU sleeps until the next event.
**In ESPET:** the slack of Core 1 in each frame. Refer to [lesson 16](./learn/16-sleep-and-battery.md).

### Xtensa
The CPU architecture (here, LX7).
**In ESPET:** CCOUNT, windowed registers, SMP atomics. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

---

## Graphics

### AABB
Axis-aligned bounding box (minimum and maximum x, y, no rotation).
**In ESPET:** the dirty rect is the union of the projected mesh and stamp AABBs, with a margin. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### Atlas
A packed image of stamps (shadow, FX). It is not the pet.
**In ESPET:** only indexed-8 stamps (shadow, FX). The pet is a skinned mesh. Index 0 = key; 1–63 actor ramps; 64–255 room. Each room has an RGB565 table of 256. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Backdrop
A full-screen photograph that is used instead of a room.
**In ESPET:** **not used.** The room is live triangles. Refer to [lesson 10](./learn/10-gravity-locked-cube.md).

### Billboard
A sprite quad that points to the camera.
**In ESPET:** **not the pet.** The shadow and the FX are stamps. The pet is skinned. The props are rigid meshes. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Blit
To copy pixels (with an optional key) onto the framebuffer.
**In ESPET:** only the shadow stamp and the FX stamps. The room, the pet and the rigids are **rasterized**. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### CASET
ST7789 "column address set": the X range of the window before RAMWR.
**In ESPET:** the dirty rect X. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### Color key
A reserved index that means "transparent".
**In ESPET:** index 0. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### COLMOD
The ST7789 color-mode command. `0x55` = RGB565.
**In ESPET:** 16 bpp on the wire. Refer to [lesson 06](./learn/06-first-pixels.md).

### Dirty rect
The smallest rectangle that changed. You must send it to the panel.
**In ESPET:** a still pose sends nothing. A full frame is already approximately 12.3 ms of wire time. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### Dodecahedron
A solid with 12 faces. Its 20 vertices are directions with equal spaces between them.
**In ESPET:** **not used.** The pet is live meshes with a continuous yaw, not photographs at 4 yaws or 20 views. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### FOV
The field of view of the perspective camera.
**In ESPET:** approximately 55°, the same in Blender and at runtime. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### FPS
Frames per second.
**In ESPET:** no locked limit. The glass limit is 80 Hz. The sim on Core 0 has its own clock. Core 1 presents up to the glass limit. Refer to [lesson 03](./learn/03-tasks-cores-timing.md).

### FX
Event particles (dust, crumbs, leaves). Point masses, not a GPU. No alpha.
**In ESPET:** an `FxPool` of 256 stamps, world `-Y`, floor or life only. No collision between particles. No triangle cards. A shake, a landing or the Hall eat makes them. Refer to [architecture 10](../architecture.md#fx).

### GRAM
The pixel RAM of the LCD controller.
**In ESPET:** it holds the last frame when you skip SPI. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### Hotspot
The pixel in a **stamp** that is put on a 3D point (shadow feet, FX center).
**In ESPET:** `hot_x, hot_y` in `SpriteRec`, only for the shadow and FX. The origin of the pet mesh is the core. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Impostor
A photograph of a 3D object that is used instead of triangles.
**In ESPET:** **not used.** The room and the pet are both triangles. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Indexed-8
Each pixel is a palette index of 1 byte, not an RGB color.
**In ESPET:** the FB and the stamps. The meshes write indices at raster time. Expand only at scanout. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### L0
An old name for a combined mesh without skin in large rooms.
**In ESPET:** **not used.** There is one skinned mesh in all rooms. The Hall and the Yard show all of the room, so the pet is small. To eat, the pet does a core squash at the Hall bowl. Refer to [architecture 1](../architecture.md#two-play-modes).

### L2
An old name for six rigid part meshes in small rooms.
**In ESPET:** **not used.** The Nest and Play show the same skinned mesh nearer. A close-up is a `full_frame` camera clip. Refer to [architecture 1](../architecture.md#two-play-modes).

### MADCTL
ST7789 memory-access control: axis flip and RGB or BGR.
**In ESPET:** set it one time at bring-up. The touch axes must agree. Refer to [lesson 06](./learn/06-first-pixels.md).

### Occluder
A mesh that the pet can walk behind (plant, furniture). It is part of the live room, not a hole in a photograph.
**In ESPET:** painter's order, from far to near, in the limit of 1024 triangles. Refer to [architecture 10](../architecture.md#room).

### Palette
A table that changes an index into RGB565.
**In ESPET:** 256 colors **for each room**. The firmware copies it on a door cut. Index 0 = key. **1–63** are the actor **ramps** (2–3 shades; they do not change between rooms). **64–255** are for this room. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### Perspective divide
After the projection, `xyz / w` gives the normalized device coordinates.
**In ESPET:** in `project(pos)`. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### Projection matrix
It changes camera-space points into clip space (FOV, near, far).
**In ESPET:** square aspect 1.0. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### RASET
ST7789 "row address set": the Y range of the window.
**In ESPET:** the dirty rect Y. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### RAMWR
ST7789 "write to GRAM": the pixel burst.
**In ESPET:** use DMA for this. Refer to [lesson 06](./learn/06-first-pixels.md).

### RGB565
A 16-bit color: 5 bits red, 6 bits green, 5 bits blue.
**In ESPET:** only on the SPI wire and in the bounce buffer. Refer to [lesson 06](./learn/06-first-pixels.md).

### Scanout
To change the CPU framebuffer into panel pixels.
**In ESPET:** indexed → RGB565 bounce → GDMA. Refer to [lesson 07](./learn/07-indexed-framebuffer.md).

### MeshRec
The record of a packed mesh (vertex offset, index offset, counts). For the pet, each vertex also has two bone IDs and two weights.
**In ESPET:** architecture §10. One skinned pet, and rigid props. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### MatRamp
Three palette indices (shadow, mid, lit) for one actor material.
**In ESPET:** N·L at triangle setup selects the band, after the skinning. The indices are in 1–63. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Painter's algorithm
Draw from far to near. Later pixels overwrite earlier pixels. No z-buffer.
**In ESPET:** from far to near over the visible triangles. No z-buffer. Refer to [architecture 10](../architecture.md#room).

### Sheet
An old name for a baked photograph of a part. **Not used for the pet.**
**In ESPET:** the appearance is live meshes. The shadow and the FX are still stamps. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Shadow
A floor stamp at the projected core on `y=0`.
**In ESPET:** it has its own `SpriteRec`. It is drawn after the room and the pet meshes. Thus, a plant can hide the pet but not the shadow on the floor. Refer to [architecture 10](../architecture.md#room).

### SpriteRec
The record of the location of a **stamp** (offset, size, hotspot).
**In ESPET:** only the shadow and the FX. The pet uses `MeshRec`. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### View matrix
The world → camera transform from `look_at`.
**In ESPET:** a 3/4 view that the room sets, never from the IMU `q`. Refer to [lesson 10](./learn/10-gravity-locked-cube.md).

### view_idx
An old painter-order slot (0…3). **Not used.** The pet turns with the bone matrices. The camera does not move until a clip sets `full_frame`. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

---

## IMU and math

### Accelerometer
A sensor for specific force. At rest, it measures gravity.
**In ESPET:** it gives down (pitch and roll). Refer to [lesson 05](./learn/05-quaternions.md).

### Bias
An offset that is slowly incorrect, usually on the gyro.
**In ESPET:** an optional `ki` integrator in the filter. Stop it when `|a|` is not approximately 1 g. Refer to [lesson 09](./learn/09-complementary-filter.md).

### Body space
The local coordinates of the pet: the core is at the origin, yaw is 0.
**In ESPET:** the core spring is in this space. Bone locals are relative to their parent. Refer to [lesson 12](./learn/12-clips-springs-hitboxes.md).

### Bone
One joint in a skeleton. After forward kinematics, it has a world 3×4 matrix.
**In ESPET:** six bones. The pose has their world matrices. Two influences for each vertex. Refer to [architecture 6](../architecture.md#6-runtime-representation).

### full_frame
A clip flag that makes all 240 rows dirty. It can move the camera, but only for that clip.
**In ESPET:** a scripted shake or close-up. IMU tilt does not set it. Refer to [architecture 1](../architecture.md#authored-camera-optional-full-frame-clip).

### Boom camera
A camera on a stick that looks at a focus point.
**In ESPET:** **not the product camera.** The habitat uses a 3/4 `look_at` that the room sets. Refer to [lesson 10](./learn/10-gravity-locked-cube.md).

### Clip
A short animation: keyframed bone-local rotations. It is not a film, and not a posed mesh for each frame.
**In ESPET:** it writes bone locals, an optional `vox_id` at the start, and an optional `full_frame`. The policy (*when* to play) is TBD. Refer to [lesson 12](./learn/12-clips-springs-hitboxes.md).

### Complementary filter
A filter that combines sensors that are good in different frequency bands (gyro for high frequencies, accelerometer for low frequencies).
**In ESPET:** `q_device_to_world` at 100 Hz. Refer to [lesson 09](./learn/09-complementary-filter.md).

### Deadband
Ignore changes that are smaller than a threshold, so that noise does not keep the system "busy".
**In ESPET:** skip SPI when the pose is the same as the last presented pose. Not `|Δq|`. Refer to [lesson 10](./learn/10-gravity-locked-cube.md).

### Cross product
A vector that is perpendicular to the two inputs.
**In ESPET:** the IMU error `a × predicted_down`; the `look_at` axes. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### Dot product
`a·b`. It measures the alignment.
**In ESPET:** N·L at triangle setup. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### Euler angles
Yaw, pitch and roll as three angles.
**In ESPET:** only for the debug overlay, not for the state. Refer to [lesson 05](./learn/05-quaternions.md).

### Gimbal lock
The loss of one degree of freedom with Euler angles at some poses.
**In ESPET:** this is why ESPET uses quaternions. Refer to [lesson 05](./learn/05-quaternions.md).

### Gyro
Gyroscope: it measures angular velocity.
**In ESPET:** it makes the tilt smooth. It is the only source of yaw. Refer to [lesson 05](./learn/05-quaternions.md).

### Hysteresis
Do not change until the new selection is better than the old selection by a margin.
**In ESPET:** not used for the draw order. The painter's sort uses depth. Refer to [lesson 11](./learn/11-sheets-and-sprites.md).

### Hitbox
A simple shape for collision. It is not the visible pixels.
**In ESPET:** spheres on the bone joints. Never mesh triangles. Refer to [lesson 12](./learn/12-clips-springs-hitboxes.md).

### Rigid body
A free object with mass, inertia and sleep: toys and props that can move.
**In ESPET:** one array, with a limit of 24 awake. The bowl: only slide and yaw. When the camera does not move, sleeping bodies are not in the scratch. Refer to [architecture 8](../architecture.md#8-physics-and-hitboxes).

### Jerk
A high-pass value, or a sudden change of acceleration.
**In ESPET:** an impulse for shake or flinch. Not the camera. Refer to [lesson 09](./learn/09-complementary-filter.md).

### look_at
A function that makes a view matrix from eye, target and up.
**In ESPET:** the room `look_at` in architecture §1. The IMU `q` is not an input. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### Magnetometer
A compass sensor.
**In ESPET:** **none**. Yaw drifts. Refer to [lesson 05](./learn/05-quaternions.md).

### Normalize
Scale a vector or a quaternion to length 1.
**In ESPET:** after each quaternion integration. Refer to [lesson 05](./learn/05-quaternions.md).

### Observability
What a set of sensors can actually find.
**In ESPET:** pitch and roll, yes; yaw around gravity, no. Refer to [lesson 05](./learn/05-quaternions.md).

### Prop
A tap magnet in a room (bowl, door, nest marker). It can also be a rigid body that can move.
**In ESPET:** a maximum of 8 for each room; screen-space pick in L rooms. The Hall bowl is the eat magnet (core squash) and a rigid that stays on the floor (slide and yaw, no flip). The Nest contents stay quiet. Refer to [architecture 1](../architecture.md#v1-map).

### Quaternion
A rotation in four numbers (`x,y,z,w` in `SharedSnap`).
**In ESPET:** `q_device_to_world`. Refer to [lesson 05](./learn/05-quaternions.md).

### Recenter
Reset the drifted yaw to a "front".
**In ESPET:** **not** a camera control. The filter yaw can still be set to zero for debug overlays. PLUS and double-tap are lizard one-shots. Refer to [lesson 09](./learn/09-complementary-filter.md).

### Rotation matrix
A 3×3 matrix (or the upper 3×3 of a 4×4 matrix) that turns vectors.
**In ESPET:** body → world for the core, then FK for the bones. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### Vector
An `(x,y,z)` point or direction.
**In ESPET:** rest, pos, cam, gravity. Refer to [lesson 04](./learn/04-vectors-matrices-camera.md).

### Verlet
A method to step positions with the previous position (or velocity). It is stable for simple physics.
**In ESPET:** the core spring against the floor. The limbs are not springs. Refer to [lesson 12](./learn/12-clips-springs-hitboxes.md).

### World space
Coordinates that are attached to the Earth. `+Y` is up.
**In ESPET:** the room and gravity. The pet does not lean with the glass. Refer to [lesson 10](./learn/10-gravity-locked-cube.md).

### Yaw
A rotation around world up.
**In ESPET:** only the gyro gives the yaw of the **device**. The habitat camera ignores it. Refer to [lesson 05](./learn/05-quaternions.md).

---

## Audio

### AEC
Acoustic echo cancellation (the microphone hears the speaker).
**In ESPET:** not in v1. Do not use the duplex graph of XiaoZhi. Refer to [lesson 15](./learn/15-audio.md).

### Codec
A chip that changes I2S numbers into analog signals (or the reverse).
**In ESPET:** the ES8311 DAC. Refer to [lesson 15](./learn/15-audio.md).

### Mixer
It adds sources into one sample stream.
**In ESPET:** no mixer task. A second class adds into the play buffer at the event. Refer to [lesson 15](./learn/15-audio.md).

### PA
The power amplifier for the speaker.
**In ESPET:** NS4150B. GPIO7 is high only while DMA runs. Refer to [lesson 15](./learn/15-audio.md).

### PCM
Stored waveform samples.
**In ESPET:** the creature chirps are s8 in flash. The sim expands them into the `int16` play buffer at the event. Impacts are procedural. No MP3, no streaming. Refer to [lesson 15](./learn/15-audio.md).

### Play buffer
One DRAM array that DMA reads while the CPU does other work.
**In ESPET:** 12 kHz mono `int16`, 300 ms, 7200 bytes. Internal, DMA-capable. Refer to [lesson 15](./learn/15-audio.md).

### Procedural audio
Sound that oscillators or noise make. It is not a wav file.
**In ESPET:** only impacts. `ImpactPatch` is 8 bytes. Refer to [lesson 15](./learn/15-audio.md).

### SfxEvt
A very small event: class, ID, velocity.
**In ESPET:** the sim renders it on that tick. A maximum of one impact and one chirp. Not a ring. Refer to [lesson 12](./learn/12-clips-springs-hitboxes.md).

### ImpactPatch
A POD that describes one thud: decay, one-pole Hz, gain, and how the velocity scales it.
**In ESPET:** 8 bytes in flash. Refer to [lesson 15](./learn/15-audio.md).

### ChirpHdr
The header of one baked creature line. `s8` samples follow it in the pak.
**In ESPET:** effort, yelp, happy, sleepy. `n` ≤ 3600. Refer to [lesson 15](./learn/15-audio.md).

### Tail
The samples that are still in front of the DMA read pointer.
**In ESPET:** sleep waits for TX-done, then the PA goes low. The limit is the 300 ms buffer. Refer to [lesson 16](./learn/16-sleep-and-battery.md).

### Voice
One class that uses the play buffer. The last event of that class replaces the previous one.
**In ESPET:** impact = procedural thud. Creature = baked chirp. Refer to [lesson 15](./learn/15-audio.md).

### vox_id
The chirp to render when a clip **starts**. 0 = no sound.
**In ESPET:** on `ClipHdr`. It is not a separate lizard trigger. Refer to [lesson 12](./learn/12-clips-springs-hitboxes.md).

---

## Power

### BAT_EN
A board net that keeps the battery power latched on.
**In ESPET:** GPIO2. Hold it high, or the board stops. Refer to [lesson 13](./learn/13-power-and-boot.md).

### CHG_STAT
The charger status pin.
**In ESPET:** GPIO3, ETA6098 STAT. Refer to [lesson 13](./learn/13-power-and-boot.md).

### Deep sleep
The CPU is off. Only the RTC and the configured pads stay on.
**In ESPET:** after TX-done, with the PA low and `BAT_EN` held. Refer to [lesson 16](./learn/16-sleep-and-battery.md).

### ETA6098
A switching Li-ion charger IC. It has no I2C map.
**In ESPET:** hardware only. You read STAT and VBAT. Refer to [lesson 13](./learn/13-power-and-boot.md).

### Light sleep
The CPU is paused and the peripherals are gated. It wakes faster than deep sleep.
**In ESPET:** only if DMA is idle and the PA is down. Refer to [lesson 16](./learn/16-sleep-and-battery.md).

### PMIC
A power-management IC with I2C (for example, AXP).
**In ESPET:** **none**. Refer to [lesson 13](./learn/13-power-and-boot.md).

### PWR
The power button.
**In ESPET:** GPIO5. A long push latches the power off. Refer to [lesson 13](./learn/13-power-and-boot.md).

### Studio mode
USB connected: bright, glass limit, no light sleep, cortex permitted.
**In ESPET:** architecture §14. Refer to [lesson 16](./learn/16-sleep-and-battery.md).

### VBAT
The battery voltage.
**In ESPET:** ADC on GPIO1 through a divider. Refer to [lesson 13](./learn/13-power-and-boot.md).

---

## ESP-IDF and toolchain

### app_main
Your entry point after the IDF initialization. It is already a FreeRTOS task.
**In ESPET:** [`firmware/main/main.c`](../firmware/main/main.c). Refer to [lesson 06](./learn/06-first-pixels.md).

### Arduino
A hobby framework for similar chips.
**In ESPET:** not used. Refer to [architecture 0](../architecture.md#0-product-lock).

### board-sim
A simulated Waveshare board on Windows: a GRAM window and a simulated IMU. It has no cube logic and no mesh rasterizer.
**In ESPET:** `sim.bat`. Refer to [lesson 00](./learn/00-start-here.md).

### Bring-up
The first hardware tests, in a fixed sequence.
**In ESPET:** architecture §15 and [guide 09](./guides/09-bring-up.md). Refer to [lesson 13](./learn/13-power-and-boot.md).

### Cortex
An optional LAN brain (UDP to an LLM on a PC). It is a guest. It is not necessary.
**In ESPET:** the radio is off by default. The behavior is TBD. Refer to [architecture 13](../architecture.md#13-optional-cortex-wi-fi-is-a-mode).

### esp_lcd
The IDF component for panels (SPI IO and an ST7789 helper).
**In ESPET:** initialization and `draw_bitmap`. You own the windows. Refer to [lesson 06](./learn/06-first-pixels.md).

### ESP-IDF
The official Espressif SDK (CMake, compilers, drivers).
**In ESPET:** 5.5 or higher, with C++ as a better C. Refer to [lesson 13](./learn/13-power-and-boot.md).

### idf.py
The IDF command line: `build`, `flash`, `monitor`.
**In ESPET:** only for the real board, not for `sim.bat`. Refer to [lesson 13](./learn/13-power-and-boot.md).

### Lizard brain
The behavior on the device (wander, wave, sleep) with the radio off.
**In ESPET:** **policy TBD**. The spatial hooks are locked (Nest clips, Hall/Yard wander, Play toys, doors, Hall eat squash, shake → dust/leaves). Do not invent a personality. Refer to [lesson 00](./learn/00-start-here.md).

### LVGL
A popular GUI library.
**In ESPET:** prohibited. Refer to [architecture 0](../architecture.md#0-product-lock).

### sdkconfig
The Kconfig output that is compiled into IDF.
**In ESPET:** octal PSRAM at 80 MHz, quad flash at 80 MHz, tick 1000, BT off. Refer to the [cheatsheet](./learn/cheatsheet.md).

### XiaoZhi
Voice-assistant firmware that some Waveshare demos include.
**In ESPET:** use only its **pins**, not the 24 kHz duplex/AEC stack. Refer to [lesson 15](./learn/15-audio.md).
