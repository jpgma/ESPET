# 14 — Touch CST816

← [13 power](./13-power-and-boot.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 15 Audio](./15-audio.md) →

**Read:** [guide 06](../guides/06-touch-cst816.md) · [CST816T register v1.3](../touch/CST816T_register_v1.3.pdf) · [CST816S register declaration](../touch/CST816S_register_declaration.pdf) (English) · [architecture poke](../../architecture.md#8-physics-and-hitboxes)

**Sim gap:** no CST816. Firmware still owns poke UV. A debug mouse/UART path from lesson 12 stays until this chip works.

## Chip

Family S/T/D. Probe `ChipID` at `0xA7`. I2C **0x15**, RST GPIO47, INT GPIO48. One finger.

Reset: RST low ~10 ms, high, wait 50–100 ms, then I2C. Datasheet timing beats blogs.

## Registers

| Addr | Use |
| :--- | :--- |
| `0x01` GestureID | `0x05` click, **`0x0B` double-click = one-shot** (lizard TBD), `0x0C` long (ignore v1) |
| `0x02` FingerNum | 0 or 1 |
| `0x03`–`0x06` XY | 12-bit. Map to UV 0…1 in 240×240. Watch MADCTL vs axes |
| `0xEC` MotionMask | bit0 `EnDClick` on |
| `0xFA` IrqCtl | pulse so GPIO48 fires. Do not poll |
| `0xFE` often DisAutoSleep | first tap eaten after idle → disable auto-sleep |

## Driver shape

Core 0, prio 11, **event**:

1. GPIO48 ISR → wake a task (no I2C in ISR).
2. Burst-read gesture + XY.
3. `0x0B` → `double_tap` one-shot in seqlock (lizard TBD; not camera recenter).
4. Else finger → `poke`, `poke_u`, `poke_v`.

Core 1 unprojects and tests spheres in S rooms (lesson 12). L rooms use screen-space slop. Touch code does not know what a nose is.

Same I2C bus as IMU: Core 0 owner serializes. Core 1 never takes the bus.

PLUS short-press is the other lizard one-shot (GPIO4). Same idea, `plus` bit — not yaw recenter.

## Checkpoint (silicon)

USB: XY + gesture. Tap → one poke, a sphere reacts (S). Double-tap → `double_tap` one-shot (log it; mapping TBD). INT is events, not 100 Hz polling. Auto-sleep does not steal the first tap (or you disabled it).

## When the board arrives

This lesson. Architecture §15 step 6 poke on real glass.

← [13 power](./13-power-and-boot.md) · [next: 15 Audio](./15-audio.md) →
