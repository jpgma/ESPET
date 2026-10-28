# 00 — Start here

[index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 01 C for firmware](./01-c-for-firmware.md) →

In this course, you will program a very small computer in a toy with the shape of a cube: the [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54). [architecture.md](../../architecture.md) describes the product. This course thinks that you have written programs before. It does **not** think that you know firmware, C or 3D math.

The **board data** (pins, schematic, ST7789, QMI8658, `BAT_EN`) is in **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). This course is about the habitat.

In Cursor, push **Ctrl** and click a link in the markdown source. Or, open **Markdown: Open Preview to the Side** (`Ctrl+Shift+V`). The Preview/Markdown switch in the tab does not open local files. (It looks in `C:\` and reports that the files are missing.)

## Two types of documents

| Folder | Reader |
| :--- | :--- |
| [learn/](./00-start-here.md) (this course) | You. Habitat concepts, the code that you write, checkpoints. |
| [jpgma/esp32-s3](https://github.com/jpgma/esp32-s3) | Silicon. Which PDF page, which register. |
| [guides/05](../guides/05-fusion-gravity-camera.md) and [09](../guides/09-bring-up.md) | Product tests on that silicon. |

When a lesson says **Read**, open those files. Do not try to read all of a datasheet.

Keep [glossary.md](../glossary.md) and [cheatsheet.md](./cheatsheet.md) open in other tabs.

## Daily procedure

1. Read the lesson. The first time that you see a `[term](../glossary.md#term)`, click it.
2. Write the code that the lesson names, in [`firmware/`](../../firmware/). **Never** put cube or pet logic in `board-sim/`.
3. Run `sim.bat` from the repository root. Make sure that the window shows the checkpoint.
4. Mark the box below. Stop when the checkpoint is true, not when you think that you are done.

The window is a simulated 240×240 panel. A **click** is a tap on a simulated [CST816](../glossary.md#cst816). A **drag** tilts a simulated [IMU](../glossary.md#imu). Your firmware must still *read* those chips and *draw* pixels. The simulator does not know what a cube is.

## What you must not invent

The [lizard-brain](../glossary.md#lizard-brain) *when* (waves, sleeps, thinks) is **TBD**. The lessons stop at the machinery: rest tables, clips, `clip_id`, springs, sound events, room swap. The spatial hooks (Nest, Play, Hall, Yard, Hall eat, FX) are locked. Do not code a personality.

## Simulator and real board

Lessons **01, 03–05, 07, 09–12** run in the ESPET `board-sim` now. The board bring-up (power, pixels, IMU registers, touch, audio, sleep) is in the other repository. Read those lessons now. Do them on silicon when the Waveshare board arrives. Each remaining build lesson has a **When the board arrives** box. Thus, you do not have to write the course again later.

The simulator has these known gaps. Write the code as the architecture says, also when the simulator does not support it:

- One thread, not two [cores](../glossary.md#core).
- No [ES8311](../glossary.md#es8311), no `BAT_EN` rail, no FIFO and IRQ, no SMP, no PSRAM and no GDMA. A click is a simulated [CST816](../glossary.md#cst816).
- The IMU is polled. A real [FIFO](../glossary.md#fifo) and [IRQ](../glossary.md#irq) come with the hardware.

To make the simulator larger is a separate job. Do not stop a lesson to wait for it.

## Progress

Fundamentals

- [ ] [01 C for firmware](./01-c-for-firmware.md)
- [ ] Buses, pins, ST7789, QMI8658 — [esp32-s3 learn](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/README.md) ([stub 02](./02-how-chips-talk.md))
- [ ] [03 Tasks, cores, timing](./03-tasks-cores-timing.md)
- [ ] [04 Vectors, matrices, camera](./04-vectors-matrices-camera.md)
- [ ] [05 Quaternions](./05-quaternions.md)

Build in the simulator

- [ ] First pixels — [esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/first-pixels.md) ([stub 06](./06-first-pixels.md))
- [ ] [07 Indexed framebuffer and dirty rect](./07-indexed-framebuffer.md)
- [ ] IMU registers — [esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/imu-registers.md) ([stub 08](./08-imu-registers.md))
- [ ] [09 Complementary filter](./09-complementary-filter.md)
- [ ] [10 Gravity-locked cube](./10-gravity-locked-cube.md) (the room sets the camera; the IMU causes a bounce, not a turn)
- [ ] [11 Skinned mesh](./11-sheets-and-sprites.md)
- [ ] [12 Clips, springs, hitboxes](./12-clips-springs-hitboxes.md)

Real board (read now)

- [ ] Power and boot — [esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/power-and-boot.md) ([stub 13](./13-power-and-boot.md))
- [ ] Touch, audio and sleep silicon — [esp32-s3 guides](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md); product tests in [guide 09](../guides/09-bring-up.md) ([stubs 14](./14-touch.md) [15](./15-audio.md) [16](./16-sleep-and-battery.md))

## If you are lost

The product in one paragraph: [architecture 0](../../architecture.md#0-product-lock).  
Pins: [cheat sheet](./cheatsheet.md) and [HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md).  
The bring-up sequence on silicon: [guide 09](../guides/09-bring-up.md).
