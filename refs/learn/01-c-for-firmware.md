# 01 — C for firmware

← [00 start](./00-start-here.md) · [index](./00-start-here.md) · [glossary](../glossary.md) · [cheat sheet](./cheatsheet.md) · [next: 02 How chips talk](./02-how-chips-talk.md) →

**Read:** [architecture 0 (style)](../../architecture.md#0-product-lock) · [firmware/main/main.c](../../firmware/main/main.c)

You will write C that is similar to a better C++: no exceptions, no RTTI, no STL in the hot path. This lesson is about the language, not the chip.

## Why firmware C is different

A desktop program can get more memory from the OS, throw an error and print a stack trace. On the ESP32, there is **no spare heap in the present loop** and no exception unwind. A bug frequently causes a reboot. You select the sizes at compile time. You check the return codes.

The ESPET style from the architecture: [POD](../glossary.md#pod) tables, integer IDs, and only the abstraction that the hardware makes necessary.

## Fixed-width types

Never use `int` alone for data that goes between chips or into a file. The size of `int` is not guaranteed.

| Type | Size | Use |
| :--- | :--- | :--- |
| `uint8_t` | 1 byte | Palette index, GPIO level, `clip_id` |
| `int16_t` | 2 bytes | Rest tracks, Q8 positions, raw IMU data |
| `uint16_t` | 2 bytes | RGB565 pixel, sprite ID |
| `uint32_t` | 4 bytes | Seqlock counter, byte offsets |
| `float` | 4 bytes | Springs, camera, quaternion |

The range of `uint8_t` is 0…255. The range of `int16_t` is approximately ±32768.

**Mental checkpoint:** if a value must fit in a register or in a `.pak` file, say its width.

## Q8 (fixed point)

You can use floats in the physics loop. The stored rest poses use **[Q8](../glossary.md#q8)**: an `int16_t` that means "value × 256".

```
float rest = stored / 256.0f;   /* Q8 → float */
int16_t stored = (int16_t)(rest * 256.0f);
```

Why use Q8? An idle table of 6 parts in Q8 is approximately 200 bytes. The same table in `float` is four times larger, and its byte order in flash is more difficult.

## Pointers and arrays

A [pointer](../glossary.md#pointer) is an address. An array name *decays* to a pointer to the first element.

```c
uint16_t s_fb[240 * 240];          /* 115200 pixels, static */
uint16_t *p = s_fb;                /* address of pixel 0 */
p[y * 240 + x] = color;            /* same as s_fb[y * 240 + x] */
```

The example above is a full RGB565 frame. [firmware/main/main.c](../../firmware/main/main.c) already uses indexed-8. Lesson 07 is about that budget.

When you send an array to a function, also send a **count**. C does not stop you when you go past the end.

## Structs, AoS, SoA

A [struct](../glossary.md#struct) puts fields together. **Array of structs** (AoS):

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

The architecture locks SoA as `Bodies`. The reason: the spring loop uses `pos`, `vel` and `rest` for all six parts. Sequential arrays use the cache better than six large objects. The [hitboxes](../glossary.md#hitbox) are on `pos[]`. There is no skeleton at runtime.

## Bits, masks, shifts

A [register](../glossary.md#register) usually has 8 bits. To set *one* bit:

```c
uint8_t ctrl7 = 0;
ctrl7 |= 0x03;        /* bits 0 and 1: accel + gyro enable */
```

`|` is OR (it sets bits). `&` is AND (it tests or clears bits). `<<` and `>>` shift.

The RGB565 packing in `main.c`:

```c
return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
```

5 bits for red, 6 for green, 5 for blue. This is a [pixel format](../glossary.md#rgb565). It is not difficult.

## Endianness

The IMU samples arrive in **little-endian** order: the low byte comes first. `le16` does this:

```c
static int16_t le16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
```

The ESP32-S3 CPU is also little-endian. Thus, this order agrees with RAM. The cortex UDP packets are also packed little-endian ([architecture 13](../../architecture.md#13-optional-cortex-wi-fi-is-a-mode)). Never send a raw `struct` with padding across the network.

## Static allocation

`s_fb` is a global (it is `static` at file scope). It stays for the full life of the program. In the frame loop, do **not** call `malloc`. Heap fragmentation in the present loop is a usual cause of a frozen toy.

`static` on a local variable in a function means "one copy that stays for the full program". It does not mean "only this function". `static` on a function or on a name at file scope means "other `.c` files cannot see it".

## `const` and headers

`const` on a table means "I will not write this". The compiler can put the table in flash ([XIP](../glossary.md#xip)).

A `.h` file **declares**. A `.c` file **defines** (the actual bytes). You can include a header from many locations. Define a global in exactly one `.c` file. ESPET will divide `firmware/` into several `.c` files. `main.c` stays the entry point.

C++ as a better C: later, you can use `constexpr`, references in helpers and `enum class`. You can **not** use exceptions, RTTI or `std::vector` in the hot path.

## Exercise (no new files yet)

Open [firmware/main/main.c](../../firmware/main/main.c). For each of these questions, write a one-line answer in a notebook (it is not necessary to write it in the file):

1. Why is `s_fb` `uint16_t` and not `uint8_t`? (Lesson 07 will change this.)
2. What does `1ull << PIN_LCD_BL` mean?
3. Why does `le16` put the low byte first?

## Checkpoint

You can read `main.c` and tell the function of each type and helper. You do not have to run new code in this lesson.

## When the board arrives

Nothing more. The C is the same.

← [00 start](./00-start-here.md) · [next: 02 How chips talk](./02-how-chips-talk.md) →
