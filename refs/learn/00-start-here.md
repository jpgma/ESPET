# 00 — Start here

[index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 01 C for firmware](./01-c-for-firmware.md) →

You are going to program a tiny computer that lives on a cube-shaped toy: the [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54). The product is described in [architecture.md](../../architecture.md). This course assumes you have written programs before, but **not** firmware, **not** C, and **not** 3D math.

**Board literacy** (pins, schematic, ST7789, QMI8658, `BAT_EN`) lives in **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). This course is the habitat.

In Cursor, **Ctrl+click** a link in the markdown source, or open **Markdown: Open Preview to the Side** (`Ctrl+Shift+V`). The Preview/Markdown toggle in the tab does not open local files (it looks under `C:\` and reports them missing).

## Two kinds of docs

| Folder | Who it is for |
| :--- | :--- |
| [learn/](./00-start-here.md) (this course) | You. Habitat concepts, code you write, checkpoints. |
| [jpgma/esp32-s3](https://github.com/jpgma/esp32-s3) | Silicon. Which PDF page, which register. |
| [guides/05](../guides/05-fusion-gravity-camera.md) and [09](../guides/09-bring-up.md) | Product tests on that silicon. |

When a lesson says **Read**, open those files. Do not try to read a datasheet cover to cover.

Keep [glossary.md](../glossary.md) and [cheatsheet.md](./cheatsheet.md) open in other tabs.

## How you work each day

1. Read the lesson. Click every `[term](../glossary.md#term)` the first time you see it.
2. Write the code it names, in [`firmware/`](../../firmware/). **Never** put cube/pet logic in `board-sim/`.
3. Run `sim.bat` from the repo root. You should see the checkpoint in the window.
4. Tick the box below. Stop when the checkpoint is true, not when you feel done.

The window is a fake 240×240 panel. Dragging it tilts a fake [IMU](../glossary.md#imu). Your firmware still has to *read* that chip and *draw* pixels. The simulator does not know what a cube is.

## What you will not invent

[Lizard-brain](../glossary.md#lizard-brain) *when* (waves, sleeps, thinks) is **TBD**. Lessons stop at the machinery: rest tables, clips, `clip_id`, springs, sound events, room swap. Spatial hooks (Nest / Play / Hall / Yard, Hall eat, FX) are locked; do not code a personality.

## Sim vs real board

Lessons **01, 03–05, 07, 09–12** run in ESPET `board-sim` today. Board bring-up (power, pixels, IMU registers, touch, audio, sleep) is the other repo; read those now, do them on silicon when the Waveshare arrives. Every remaining build lesson still has a **When the board arrives** box so you do not rewrite the course later.

Known sim gaps (write the code the architecture way anyway):

- One thread, not two [cores](../glossary.md#core).
- No [CST816](../glossary.md#cst816), [ES8311](../glossary.md#es8311), or `BAT_EN`.
- IMU is polled; a real [FIFO](../glossary.md#fifo) + [IRQ](../glossary.md#irq) comes on hardware.

Growing the simulator is a separate job. Do not stall a lesson waiting for it.

## Progress

Fundamentals

- [ ] [01 C for firmware](./01-c-for-firmware.md)
- [ ] Buses, pins, ST7789, QMI8658 — [esp32-s3 learn](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/README.md) ([stub 02](./02-how-chips-talk.md))
- [ ] [03 Tasks, cores, timing](./03-tasks-cores-timing.md)
- [ ] [04 Vectors, matrices, camera](./04-vectors-matrices-camera.md)
- [ ] [05 Quaternions](./05-quaternions.md)

Build in sim

- [ ] First pixels — [esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/first-pixels.md) ([stub 06](./06-first-pixels.md))
- [ ] [07 Indexed framebuffer and dirty rect](./07-indexed-framebuffer.md)
- [ ] IMU registers — [esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/imu-registers.md) ([stub 08](./08-imu-registers.md))
- [ ] [09 Complementary filter](./09-complementary-filter.md)
- [ ] [10 Gravity-locked cube](./10-gravity-locked-cube.md) (authored room; IMU bounce, not orbit)
- [ ] [11 Skinned mesh](./11-sheets-and-sprites.md)
- [ ] [12 Clips, springs, hitboxes](./12-clips-springs-hitboxes.md)

Real board (read now)

- [ ] Power and boot — [esp32-s3](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/learn/power-and-boot.md) ([stub 13](./13-power-and-boot.md))
- [ ] Touch / audio / sleep silicon — [esp32-s3 guides](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/docs/guides/09-bring-up.md); product tests [guide 09](../guides/09-bring-up.md) ([stubs 14](./14-touch.md) [15](./15-audio.md) [16](./16-sleep-and-battery.md))

## If you get lost

Product in one paragraph: [architecture 0](../../architecture.md#0-product-lock).  
Pins: [cheat sheet](./cheatsheet.md) and [HARDWARE.md](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md).  
Bring-up order on silicon: [guide 09](../guides/09-bring-up.md).
