# 08 — IMU registers

← [07 FB](./07-indexed-framebuffer.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 09 Filter](./09-complementary-filter.md) →

**Read:** [guide 04](../guides/04-imu-qmi8658.md) · [QMI8658A datasheet](../imu/QMI8658A_Datasheet_Rev_A.pdf) (WHO_AM_I, CTRL2/3/7, FIFO_* ) · [architecture 4 Core 0 prio 12](../../architecture.md#4-core-allocation)

**Code:** keep I2C in `firmware/`. Split a `imu.c` if `main.c` is getting loud. Still **no** cube.

## Transactions

Write: `[reg][value…]` via `i2c_master_transmit`.  
Read: `transmit_receive(reg, n)`. Address **0x6B** in current hello (`SA0` low). If `WHO_AM_I` fails, try **0x6A**.

Expect `WHO_AM_I == 0x05` on this board/sim ([board_sim.h](../../board-sim/include/board_sim.h)). Log it. If silicon differs, log and continue only if the map still matches.

## Bring-up sequence (A datasheet)

1. Soft reset if the sheet has a CTRL9 command for it; wait.
2. `WHO_AM_I`.
3. `CTRL2` accel: ODR + full scale (start ±8 g so shakes do not clip; sensitivity [LSB/g](../glossary.md#lsb) from the table — hello uses 4096).
4. `CTRL3` gyro: ODR + dps (start ±512 or ±1024).
5. `CTRL7` = accel|gyro enable (`0x03` in hello).
6. Later: `FIFO_WTM_TH`, `FIFO_CTRL`, map INT to GPIO6.

Sim today: **polling** `AX_L` + 12 bytes is OK. The fake chip is not a full FIFO. Write the register names as constants anyway so silicon can switch.

**Do not** I2C from an ISR. **Do not** init ES8311 in this task.

## Scaling

Raw `int16_t` / `LSB_PER_G` → g. Gyro / datasheet LSB per dps → deg/s, then × π/180 to rad/s for the filter.

`le16` is little-endian (lesson 01).

## Axis mapping (experiment)

Datasheet axes are the *package*, not “screen up.” Hold the device (or drag the sim):

- Rest, screen toward you / window facing you: one accel axis ≈ ±1 g.
- Tilt top edge toward you: another axis grows.

Write a table in a comment: `device_x → ?`. Camera `forward = rotate(q, {0,0,-1})` is confirmed in lesson 10, not guessed here.

Print `ax ay az gx gy gz` at ~10 Hz in the log so you are not staring at pixels only.

## FIFO + watermark (read now, code on silicon)

[FIFO](../glossary.md#fifo): the chip stores N samples. [Watermark](../glossary.md#watermark) INT (GPIO6) fires when enough are ready. Core 0 prio 12 drains, runs the filter, publishes the seqlock. Target **100 Hz** out.

Overflow bit set → you were too slow. Never drop the drain to “whenever the 30 Hz loop feels like it” on silicon.

## Checkpoint (sim)

Drag: accel vector moves, gyro spikes while moving, quiets when you stop. `WHO_AM_I` stays 0x05. Indexed screen can still tint from `ax,ay,az` (hello already does this).

## When the board arrives

100 Hz print, `|a| ≈ 1` at rest, FIFO overflow count 0, INT rate ~100 Hz. [Guide 04](../guides/04-imu-qmi8658.md) test.

← [07 FB](./07-indexed-framebuffer.md) · [next: 09 Filter](./09-complementary-filter.md) →
