# 06 — Touch CST816

**Goal:** IRQ on GPIO48 → one finger UV poke; double-tap → recenter. Nothing else in v1.

Waveshare says “CST816”; the resources pack is **CST816T**. S/T/D share the register map closely enough. Probe `ChipID` at `0xA7` and log it.

## PDFs

| File | Read |
| :--- | :--- |
| [`../touch/CST816T_register_v1.3.pdf`](../touch/CST816T_register_v1.3.pdf) | **Programming.** GestureID, XY, IrqCtl, MotionMask, auto-sleep, reset |
| [`../touch/CST816S_register_declaration.pdf`](../touch/CST816S_register_declaration.pdf) | Same map in English (short) |
| [`../touch/CST816S_Datasheet_EN.pdf`](../touch/CST816S_Datasheet_EN.pdf) | Reset timing, IRQ, sleep, I2C 10–400 kHz |
| [`../touch/CST816D_datasheet_En_V1.3.pdf`](../touch/CST816D_datasheet_En_V1.3.pdf) | Extra electrical if T/S disagree with the glass |

I2C **0x15**. RST GPIO47, INT GPIO48.

## Registers you implement

| Name | Addr | ESPET use |
| :--- | :--- | :--- |
| GestureID | `0x01` | `0x00` none, `0x05` click, **`0x0B` double-click** = recenter, `0x0C` long press (ignore in v1) |
| FingerNum | `0x02` | 0 or 1. Architecture is one finger |
| XH/XL YH/YL | `0x03`–`0x06` | 12-bit coords. Map to UV in 240×240. Watch MADCTL vs touch axes |
| ChipID | `0xA7` | Log at boot |
| MotionMask | `0xEC` | Bit0 `EnDClick` **on** |
| IrqCtl | `0xFA` | Pulse on change / motion so GPIO48 fires. Do not poll |
| DisAutoSleep | often `0xFE` | If the first tap after idle is eaten, disable auto-sleep |

Reset pulse (typical family): RST low ~10 ms, high, wait ~50–100 ms, then I2C. Datasheet timing beats folklore.

## Driver shape

Core 0, prio 11, event-driven:

1. GPIO48 ISR → wake a small task (do not I2C inside the ISR).
2. Read gesture + XY in one burst.
3. If `0x0B`: set `recenter` one-shot in the seqlock.
4. Else if finger down: set `poke`, `poke_u`, `poke_v`.

Core 1 unprojects the poke through `inv(proj*view)`, ray vs spheres (`architecture.md` §8). Touch code does **not** know what a nose is.

Same I2C bus as the IMU: the owner task serializes. Never take the bus from Core 1.

## What not to do

- LVGL indev.
- Two-finger gestures (chip may advertise them; product is one finger).
- PLUS as volume. PLUS is recenter, same as double-tap.

## Bring-up test

Print XY + gesture on USB. Tap: one `poke`. Double-tap: `recenter` and the cube yaw jumps to “front” (after fusion exists). Confirm INT rate is events, not 100 Hz polling.
