# 16 — Sleep and battery

← [15 audio](./15-audio.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [00 start](./00-start-here.md)

**Read:** [guide 08](../guides/08-power-battery.md) · [architecture 14](../../architecture.md#14-power-8-h-strive) · IDF sleep + PM (linked from the guide) · [architecture 4 slack](../../architecture.md#4-core-allocation)

Strive ~8 h dim, radio off, ~1000 mAh → ~125 mA average. Not a cap that kills chirps. Measure; do not guess.

## Three different “idles”

| State | Display | Audio | CPU |
| :--- | :--- | :--- | :--- |
| Interacting | 30 Hz dirty | PA as needed | 240 MHz |
| Idle, mixer live | [GRAM-hold](../glossary.md#gram) | tail, PA high | Core 1 [WFI](../glossary.md#wfi); **no** chip light-sleep |
| Idle, mixer silent | GRAM-hold | PA low, I2S clocks off, codec standby | [Light sleep](../glossary.md#light-sleep) allowed |
| Face-down / PWR / long idle | off or GRAM | finish tail, PA low, then [deep sleep](../glossary.md#deep-sleep) | `BAT_EN` held, [RTC SRAM](../glossary.md#rtc-sram) restore |

Golden rule 4: spinning at 240 MHz with a static frame is a bug. Silence with I2S clocks or PA up is the same bug.

Light-sleep **only if mixer idle**. USB Serial/JTAG dies in sleep.

## RTC SRAM

Hunger, happy, sleep, last emotion (later `part_ids[6]`). Decay rates TBD with the lizard brain. The storage exists now. Deep sleep restores these; it does not restore DRAM framebuffers (GRAM may still show the last face until reset — know what your panel does).

`BAT_EN` uses RTC GPIO hold so it stays high while the CPU is dead.

## Levers

- Backlight PWM 30–40% battery; bump on poke, decay.
- Dirty ST7789 only.
- CPU 240 MHz interacting; 80/160 after a few seconds still.
- Wi-Fi off unless cortex.
- ES7210 off, SD off.
- Deadband on `|Δq|` / `|Δcam|` or 8 h dies from noise-driven 30 Hz SPI.

## Cortex (last)

Radio off by default. UDP inbox on LAN, not JSON in the frame path. Failure → lizard. Architecture §15 step 9. Not a sleep feature; listed so you do not turn Wi-Fi on “to test” while measuring mA.

## Checkpoint (silicon)

Face-down: no new `SfxEvt`, tail ends, PA low, sleep, wake restores RTC fields. Unplug USB: backlight personality changes. Current draw in idle (radio off, GRAM-hold, PA low) is something you write down, not a hope.

Architecture §15 step 7 + 8 tail/sleep bullets + [guide 09](../guides/09-bring-up.md) last rows.

You now have the machinery. Policy (wander, wave, think) stays TBD.

← [15 audio](./15-audio.md) · [00 start](./00-start-here.md)
