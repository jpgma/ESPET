# 15 — Audio ES8311 + mixer

← [14 touch](./14-touch.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 16 Sleep](./16-sleep-and-battery.md) →

**Read:** [guide 07](../guides/07-audio-es8311.md) · [ES8311 user guide](../audio/ES8311.user.Guide.pdf) (clocks, SDP `0x09`/`0x0A`) · [ES8311.DS.pdf](../audio/ES8311.DS.pdf) (register list) · [NS4150B.pdf](../audio/NS4150B.pdf) (CTRL timings) · [architecture 9](../../architecture.md#9-audio-procedural-two-voices)

**Sim gap:** no codec, no speaker. You can still implement the mixer into a DRAM bounce and log peaks. Do not block Core 1.

**Speaker:** MX1.25 header, confirm when the board arrives. Open header is not a missing driver.

## Locked v1

- Core 0 mixer, **two voices**, procedural patches, **no PCM**, no TF, no MP3.
- 12 kHz mono, block 256 samples, I2S GDMA → ES8311.
- [ES7210](../glossary.md#es7210) **off**. DIN GPIO11 unused.
- Volume fixed. PLUS is not volume (lizard one-shot).
- Core 1 never plays audio. It emits `SfxEvt`.

Steal pins, not XiaoZhi’s 24 kHz duplex/AEC stack.

## ES8311

I2C `0x18`. MCLK 8, BCLK 9, WS 10, DOUT 12.

| Reg | Job |
| :--- | :--- |
| `0x00` | reset |
| `0x01`–`0x03` | clocks: MCLK on, ADC clocks **off**, DAC on, dividers for 12 kHz |
| `0x09` | SDP in: I2S, 16-bit, unmute |

Lazy-init on first sound. **Standby between phrases** (re-init is tens of ms — a wall hit misses). Full off only on deep sleep.

I2C writes only on start/stop/volume — **never** inside the 100 Hz IMU drain.

Read Espressif `es8311` init for the sequence, then write your own 12 kHz path.

## NS4150B [PA](../glossary.md#pa)

GPIO7 CTRL. High **only while a voice is live**.

Typical: `Twk` ~35 ms wake, `Tst` ~120 ms cold start, `Tsd` ~80 ms shutdown, `ISD` a few µA when off. Raise GPIO7 *before* the first sample or you clip the attack. Silence with Class-D still clocked is a hiss and a battery leak.

## Mixer

```
Core 1 collide → SfxEvt {id, vel, tag}   // voice A
Core 0 clip start (vox_id) / jerk        // voice B
A: impact (noise + bandpass)
B: creature (2-op FM or square+noise)
int32 mix → sat int16 → I2S
```

`SynthPatch` 16 bytes, copy flash → DRAM at boot. Last-event-wins inside a class. Two voices layer (wave + bop). Idle ambient loops **off**.

`ClipHdr.vox_id` plays on clip **start**. Emotion tables never auto-vox. `vox_id == 0` silent.

## Checkpoint (silicon)

1. I2C ACK `0x18`.
2. PA pulse you can hear (proves GPIO7 + speaker).
3. 12 kHz sine, then PA low — actually silent.
4. Debug poke → impact (A). Clip with `vox_id` + toy hit → two voices.
5. GRAM-hold while a tail plays; PA drops after.

Architecture §15 step 8.

← [14 touch](./14-touch.md) · [next: 16 Sleep](./16-sleep-and-battery.md) →
