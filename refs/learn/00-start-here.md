# 00 — Start here

[index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 01 C for firmware](01-c-for-firmware.md) →

You are going to program a tiny computer that lives on a cube-shaped toy: the [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54). The product is described in [architecture.md](../../architecture.md). This course assumes you have written programs before, but **not** firmware, **not** C, and **not** 3D math.

## Two kinds of docs

| Folder | Who it is for |
| :--- | :--- |
| [learn/](00-start-here.md) (this course) | You. Concepts, vocabulary, code you write, checkpoints. |
| [guides/](../guides/01-board-and-pins.md) | Future you. Blunt “which PDF page, which register.” No teaching. |

When a lesson says **Read**, open those files. Do not try to read a datasheet cover to cover.

Keep [glossary.md](../glossary.md) and [cheatsheet.md](cheatsheet.md) open in other tabs.

## How you work each day

1. Read the lesson. Click every `[term](../glossary.md#term)` the first time you see it.
2. Write the code it names, in [`firmware/`](../../firmware/). **Never** put cube/pet logic in `board-sim/`.
3. Run `sim.bat` from the repo root. You should see the checkpoint in the window.
4. Tick the box below. Stop when the checkpoint is true, not when you feel done.

The window is a fake 240×240 panel. Dragging it tilts a fake [IMU](../glossary.md#imu). Your firmware still has to *read* that chip and *draw* pixels. The simulator does not know what a cube is.

## What you will not invent

[Lizard-brain](../glossary.md#lizard-brain) behaviour (when the pet waves, sleeps, thinks) is **TBD** in the architecture. Lessons stop at the machinery: rest tables, clips, `clip_id`, springs, sound events. Do not code a personality.

## Sim vs real board

Lessons **01–12** run in `board-sim` today. Lessons **13–16** you *read now* and *do on silicon* when the Waveshare arrives. Every build lesson still has a **When the board arrives** box so you do not rewrite the course later.

Known sim gaps (write the code the architecture way anyway):

- One thread, not two [cores](../glossary.md#core).
- No [CST816](../glossary.md#cst816), [ES8311](../glossary.md#es8311), or `BAT_EN`.
- IMU is polled; a real [FIFO](../glossary.md#fifo) + [IRQ](../glossary.md#irq) comes on hardware (lesson 08 / 13).

Growing the simulator is a separate job. Do not stall a lesson waiting for it.

## Progress

Fundamentals

- [ ] [01 C for firmware](01-c-for-firmware.md)
- [ ] [02 How chips talk](02-how-chips-talk.md)
- [ ] [03 Tasks, cores, timing](03-tasks-cores-timing.md)
- [ ] [04 Vectors, matrices, camera](04-vectors-matrices-camera.md)
- [ ] [05 Quaternions](05-quaternions.md)

Build in sim

- [ ] [06 First pixels](06-first-pixels.md)
- [ ] [07 Indexed framebuffer and dirty rect](07-indexed-framebuffer.md)
- [ ] [08 IMU registers](08-imu-registers.md)
- [ ] [09 Complementary filter](09-complementary-filter.md)
- [ ] [10 Gravity-locked cube](10-gravity-locked-cube.md)
- [ ] [11 Sheets and sprites](11-sheets-and-sprites.md)
- [ ] [12 Clips, springs, hitboxes](12-clips-springs-hitboxes.md)

Real board (read now)

- [ ] [13 Power and boot](13-power-and-boot.md)
- [ ] [14 Touch](14-touch.md)
- [ ] [15 Audio](15-audio.md)
- [ ] [16 Sleep and battery](16-sleep-and-battery.md)

## If you get lost

Product in one paragraph: [architecture 0](../../architecture.md#0-product-lock).  
Pins: [cheat sheet](cheatsheet.md).  
Bring-up order on silicon: [guide 09](../guides/09-bring-up.md).
