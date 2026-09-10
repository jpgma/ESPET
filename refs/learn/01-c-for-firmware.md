# 01 — C for firmware

← [00 start](00-start-here.md) · [index](00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](cheatsheet.md) · [next: 02 How chips talk](02-how-chips-talk.md) →

**Read:** [architecture 0 (style)](../../architecture.md#0-product-lock) · [firmware/main.c](../../firmware/main.c)

You will write C that looks like a better C++: no exceptions, no RTTI, no STL in the hot path. This lesson is the language, not the chip.

## Why firmware C feels different

A desktop program can ask the OS for more memory, throw an error, and print a stack trace. On the ESP32 there is **no spare heap in the 33 ms loop**, no exception unwind, and a bug often means a reboot. You pick sizes at compile time. You check return codes.

ESPET style from architecture: [POD](../glossary.md#pod) tables, integer IDs, as little abstraction as the hardware forces.

## Fixed-width types

Never use bare `int` for data that crosses a chip or a file. The size of `int` is not a promise.

| Type | Size | Use |
| :--- | :--- | :--- |
| `uint8_t` | 1 byte | palette index, GPIO level, `clip_id` |
| `int16_t` | 2 bytes | rest tracks, Q8 positions, IMU raw |
| `uint16_t` | 2 bytes | RGB565 pixel, sprite id |
| `uint32_t` | 4 bytes | seqlock counter, byte offsets |
| `float` | 4 bytes | springs, camera, quaternion |

`uint8_t` is 0…255. `int16_t` is about ±32768.

**Checkpoint in your head:** if a value must fit in a register or a `.pak` file, name the width.

## Q8 (fixed point)

Floats are fine in the physics loop. Stored rest poses use **[Q8](../glossary.md#q8)**: an `int16_t` that means “value × 256”.

```
float rest = stored / 256.0f;   /* Q8 → float */
int16_t stored = (int16_t)(rest * 256.0f);
```

Why bother? A 6-part idle table in Q8 is ~200 bytes. Same table in `float` is four times bigger and endian-messy on flash.

## Pointers and arrays

A [pointer](../glossary.md#pointer) is an address. An array name *decays* to a pointer to the first element.

```c
uint16_t s_fb[240 * 240];          /* 115200 pixels, static */
uint16_t *p = s_fb;                /* address of pixel 0 */
p[y * 240 + x] = color;            /* same as s_fb[y * 240 + x] */
```

`s_fb` in [firmware/main.c](../../firmware/main.c) is a full RGB565 frame. Lesson 07 will shrink that to indexed-8.

If you pass an array to a function, you also pass a **count**. C will not stop you from walking off the end.

## Structs, AoS, SoA

A [struct](../glossary.md#struct) groups fields. **Array of structs** (AoS):

```c
typedef struct { float pos[3]; float vel[3]; } Body;
Body bodies[6];
```

**Struct of arrays** ([SoA](../glossary.md#soa)):

```c
typedef struct {
    float pos[6][3];
    float vel[6][3];
    float rest[6][3];
} Bodies;
```

Architecture locks SoA as `Bodies`. Why: the spring loop touches `pos`, `vel`, `rest` for all six parts. Sequential arrays cache better than six fat objects. [Hitboxes](../glossary.md#hitbox) live on `pos[]`. There is no skeleton at runtime.

## Bits, masks, shifts

A [register](../glossary.md#register) is usually 8 bits. To turn *one* bit on:

```c
uint8_t ctrl7 = 0;
ctrl7 |= 0x03;        /* bits 0 and 1: accel + gyro enable */
```

`|` is OR (set bits). `&` is AND (test or clear). `<<` / `>>` shift.

RGB565 packing in `main.c`:

```c
return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
```

5 bits red, 6 green, 5 blue. That is a [pixel format](../glossary.md#rgb565), not a mystery.

## Endianness

IMU samples arrive **little-endian**: low byte first. That is what `le16` does:

```c
static int16_t le16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
```

The ESP32-S3 CPU is also little-endian, so this matches RAM. The cortex UDP packets are packed little-endian too ([architecture 13](../../architecture.md#13-optional-cortex-wi-fi-is-a-mode)). Never send a raw `struct` with padding across the network.

## Static allocation

`s_fb` is a global (actually `static` at file scope). It lives for the whole program. In the frame loop you do **not** call `malloc`. Heap fragmentation plus a 33 ms deadline is how toys freeze.

`static` on a function-local variable means “one copy, lives forever,” not “this function only.” `static` on a function or file-scope name means “not visible to other `.c` files.”

## `const` and headers

`const` on a table means “I will not write this.” The compiler can put it in flash ([XIP](../glossary.md#xip)).

A `.h` file **declares**. A `.c` file **defines** (the actual bytes). Include a header from many places; define a global in exactly one `.c`. ESPET will grow `firmware/` into several `.c` files; `main.c` stays the entry.

C++ as better C: you may use `constexpr`, references in helpers, and `enum class` later. You may **not** use exceptions, RTTI, or `std::vector` in the hot path.

## Exercise (no new files yet)

Open [firmware/main.c](../../firmware/main.c). For each of these, write a one-line comment in a notebook (not necessarily in the file):

1. Why is `s_fb` `uint16_t` and not `uint8_t`? (Lesson 07 will change this.)
2. What does `1ull << PIN_LCD_BL` mean?
3. Why does `le16` or the low byte first?

## Checkpoint

You can read `main.c` and say what every type and helper is for. You do not need to run anything new this lesson.

## When the board arrives

Nothing extra. Same C.

← [00 start](00-start-here.md) · [next: 02 How chips talk](02-how-chips-talk.md) →
