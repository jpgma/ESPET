# 07 — Audio ES8311 + NS4150B (v1)

**Goal:** 12 kHz mono I2S TX, two procedural voices on Core 0, PA high only while a voice is live. ES7210 off. No PCM, no TF, no AEC.

## PDFs

| File | Read |
| :--- | :--- |
| [`../audio/ES8311.user.Guide.pdf`](../audio/ES8311.user.Guide.pdf) | **Programming.** Clock tree, MCLK/BCLK dividers, SDP `0x09`/`0x0A` (I2S, 16-bit), DAC path, mute, standby vs power-down |
| [`../audio/ES8311.DS.pdf`](../audio/ES8311.DS.pdf) | Electrical + full register list. Use as the map; the user guide is the narrative |
| [`../audio/NS4150B.pdf`](../audio/NS4150B.pdf) | CTRL pin, shutdown current, **start/wake/shutdown times** |
| [`../audio/ES7210-datasheet.pdf`](../audio/ES7210-datasheet.pdf) | **Do not init.** Know it exists on the bus so you do not accidentally probe it |

IDF: [I2S](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/i2s.html)

Pins: MCLK 8, BCLK 9, WS 10, DIN 11 **unused v1**, DOUT 12. PA GPIO7. Codec I2C on 41/42.

Speaker: MX1.25, non-polarized, **TBD at bring-up**. Open header ≠ missing driver.

## ES8311 — what to lock

v1: **DAC only**, I2S slave or master — pick one and match IDF. Architecture assumes the S3 provides MCLK (GPIO8).

From the user guide, the registers that decide whether you get a sine or silence:

| Reg | Job |
| :--- | :--- |
| `0x00` | Reset |
| `0x01`–`0x03` | Clock manager: MCLK on, ADC clocks **off**, DAC clocks on, dividers for **12 kHz** |
| `0x09` | SDP in: I2S, 16-bit (`WL=011`), unmute, left data to DAC |
| DAC analog / volume | Fixed volume in v1 (studio may be louder, like backlight) |
| Standby vs power-down | **Standby between phrases.** Full off only on deep sleep. Re-init is tens of ms — a wall hit would miss |

12 kHz, not XiaoZhi’s 24 kHz. Confirm with a sine at bring-up; drop to 8 kHz only if the amp/coil is ugly.

I2C writes: play start / stop / volume only. Never inside the 100 Hz IMU drain.

Espressif `esp_codec_dev` / `es8311` component: **read the register sequence**, then write your own 12 kHz path. Do not take their 24 kHz duplex graph.

## Mixer (not in any datasheet)

Architecture §9:

```
Core 1 collide → SfxEvt {id, vel, tag}     // voice A, never blocks
Core 0 clip start / jerk  → mixer poke     // voice B

12 kHz, block 256:
  A: impact (noise + bandpass, decay from vel)
  B: creature (2-op FM or square+noise)
  int32 mix → sat int16 → I2S GDMA → ES8311
```

Patches are 16-byte PODs in DRAM. Hot path never reads flash for audio. Two voices, last-event-wins inside a class.

Silence with I2S clocks running **or** PA up is the audio version of a static 30 Hz SPI.

## NS4150B — GPIO, not I2C

CTRL = GPIO7.

From the Nsiway sheet (typical; confirm in the PDF):

| Param | Typical | Consequence |
| :--- | :--- | :--- |
| `ISD` CTRL=0 | 1–10 µA | PA **must** drop between phrases or 8 h dies |
| `Tst` start | ~120 ms | First chirp after cold PA is late. Keep codec in standby; PA wake ~35 ms (`Twk`) is the usual path |
| `Twk` wake | ~35 ms | Raise GPIO7 **before** the first sample of a phrase, or you clip the attack |
| `Tsd` shutdown | ~80 ms | Do not re-trigger during this if you need a clean gate |

High only while a voice is live. Let the tail finish (~800 ms cap) before light-sleep or PA low. Face-down: do not start new events; finish tail; PA low; then sleep.

## ES7210

On the schematic, on the I2C bus, mics exist. v1: do not call its init, do not enable I2S RX, DIN pin unused. Mic later is a new task, not a silent dependency.

## Bring-up test (step 8)

1. I2C ACK on 0x18.
2. PA pulse you can hear as a pop (proves GPIO7 and the speaker header).
3. 12 kHz sine, PA high, then PA low — confirm silence is actually silent (scope or ear: no hiss from an idle Class-D).
4. Debug poke → impact patch (voice A). Clip with `vox_id` + toy hit → two voices.
5. GRAM-hold while a tail plays; PA drops after; face-down waits for mixer idle.
