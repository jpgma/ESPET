---
name: Waveshare board repo
overview: Create a private GitHub repo `jpgma/esp32-s3` as an ESP32-S3 family library, with this Waveshare SKU nested so more boards can land later. Peel ESPET down to a thin pin/budget table plus product docs that link out.
todos:
  - id: scaffold-repo
    content: Create sibling repo F:/Repos/jpgma/esp32-s3 with family README, LICENSE, NOTICE, .gitignore, and boards/waveshare-touch-lcd-154/
    status: completed
  - id: hardware-bible
    content: Write boards/waveshare-touch-lcd-154/HARDWARE.md (exists / does-not-exist / numbers / testbed choices vs project policy) plus a short family docs/soc.md
    status: completed
  - id: copy-rewrite-docs
    content: Copy PDFs (soc at repo root, SKU with the board); rewrite guides 01-04/06-08, silicon bring-up, hardware glossary/cheatsheet, short board learn lessons
    status: completed
  - id: copy-sim-hello
    content: Copy board-sim + firmware hello under the SKU folder; de-ESPET names; sim.bat from that folder
    status: completed
  - id: github-private
    content: Create private github.com/jpgma/esp32-s3 and push
    status: completed
  - id: strip-espet
    content: Remove ESPET refs PDFs/hardware guides/learn; thin architecture §0/§2 table; point README/learn/guides 05+09 at jpgma/esp32-s3 (this SKU path)
    status: completed
isProject: false
---

# Peel Waveshare hardware into jpgma/esp32-s3

## What this is

A **new** private repo, sibling to ESPET: `F:/Repos/jpgma/esp32-s3` → `https://github.com/jpgma/esp32-s3`.

It is an **ESP32-S3 family library**, not a single-SKU dump. Day one only contains one board — Waveshare ESP32-S3-Touch-LCD-1.54 — nested so a second SKU later is an add, not a reshuffle. ESPET stays the habitat product: it keeps its own `board-sim`/`firmware` daily loop (no submodule) and **stops hosting** the hardware library.

This SKU’s testbed stack (stated up front, not inherited from Waveshare’s demo): ESP-IDF ≥ 5.5, C as IDF C, **not** Arduino / LVGL / XiaoZhi / 24 kHz duplex. Steal **pins and register init** from [waveshareteam/ESP32-S3-Touch-LCD-1.54](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.54) only.

## New repo layout

Nest the SKU now. SoC manuals and S3-wide facts live at the family root; pins, schematic, and the fake panel live under the board.

```text
esp32-s3/
  README.md                         # family index: which boards exist, how to start
  LICENSE                           # MIT for original code + markdown
  NOTICE                            # vendor PDFs remain Espressif/Waveshare/QST/etc.
  docs/soc.md                       # S3-wide law (cores, PSRAM modes, Wi-Fi DMA, errata)
  refs/soc/                         # Espressif S3 datasheet, TRM, errata, HW guidelines
  boards/
    waveshare-touch-lcd-154/
      README.md                     # this SKU: sim.bat, hello, wiki link
      HARDWARE.md                   # SKU contract: exists / does-not-exist / numbers
      sim.bat  debug.bat
      firmware/                     # hello LCD + QMI8658
      board-sim/                    # fake 240×240 ST7789 + mouse IMU
      docs/
        SOURCES.md                  # this board’s wiki, demo repo, chip PDFs
        glossary.md                 # buses + this board’s chips
        cheatsheet.md               # pins, I2C, ST7789, QMI8658
        guides/                     # blunt PDF-page notes, product-neutral
        learn/                      # short board literacy
      refs/                         # schematic + ST7789, QMI8658, CST816, ES8311, ETA6098, W25Q128
```

Do **not** copy [`architecture.md`](architecture.md), habitat learn 04/05/07/09–12, or `.cursor/plans/`. New git history (not a filter-branch of ESPET).

Family vs SKU (so a later board does not inherit this glass):

- **Family (`docs/soc.md`, `refs/soc/`):** dual LX7, 512 KB SRAM, octal vs quad PSRAM, Wi-Fi DMA cannot live in PSRAM, native USB, strapping-pin *concept*, IDF ≥ 5.5.
- **SKU (`boards/waveshare-touch-lcd-154/`):** 8 MB octal / 16 MB quad as *this module*, 240×240 ST7789, QMI8658, CST816, ES8311, `BAT_EN` GPIO2, the GPIO table, schematic. A future board gets its own `HARDWARE.md` and must not reuse this pin map.

## SKU HARDWARE.md is the starting contract

Every project on **this glass** reads `boards/waveshare-touch-lcd-154/HARDWARE.md` before writing pixels. Split **silicon/SKU law** from **project policy**. S3-wide facts are summarized there and owned in `docs/soc.md`.

**Exists (chips / buses)**

- ESP32-S3R8, dual LX7 @ 240 MHz, 512 KB SRAM, 8 MB in-package **octal** PSRAM @ 80 MHz, 16 MB **quad** NOR (W25Q128)
- 1.54" 240×240 IPS, ST7789, 4-wire SPI (CS 21, CLK 38, MOSI 39, DC 45, RST 40, BL 46), host SPI3
- QMI8658 accel+gyro, INT GPIO6, I2C 0x6A/0x6B, `WHO_AM_I` 0x05
- CST816-family touch, I2C 0x15, INT 48, RST 47, **one finger**
- ES8311 DAC 0x18, I2S 8/9/10/11/12, NS4150B PA GPIO7, ES7210 on the bus
- ETA6098 charger (no I2C), `BAT_EN` GPIO2, VBAT GPIO1, `CHG_STAT` GPIO3, PWR/PLUS/BOOT 5/4/0
- Native USB on 19/20 (no CH340). One I2C pair 41/42 for four slaves. TF slot present.

**Does not exist (do not design as if they do)**

- LCD **TE** / vsync; LCD **MISO**
- Magnetometer → **no yaw around gravity** from this IMU
- USB-UART bridge; AXP-style PMIC / I2C fuel gauge
- Second I2C controller in the netlist; speaker is an **MX1.25 header**, not a proven driver

**Hard numbers (physics, not taste)**

- Full-frame RGB565 = 115200 bytes. @ 40 MHz SPI ≈ 23 ms; try 80 MHz, fall back 40. Mode 0 first, confirm 0 vs 3 on glass.
- GPIO45 (DC) and GPIO46 (BL) are **strapping**.
- Wi-Fi DMA **cannot** live in PSRAM (S3 fact).
- `BAT_EN` must be held high or the board dies on battery.
- Shared I2C: **one software owner**. Codec init vs 100 Hz IMU drain will stall the filter if they fight.

**This testbed’s choices (labeled as such, override per project)**

- Hello inits LCD + IMU poll only. Does **not** init ES7210, TF, Wi-Fi, Bluetooth.
- `board-sim` fakes GRAM + QMI8658 (mouse tilt). It does **not** fake dual-core SMP, CST816, ES8311, `BAT_EN`, FIFO IRQ, octal PSRAM, or GDMA.

ESPET-only policy stays **out** of this file: indexed-8, 30 FPS as a product cap, 2-voice 12 kHz mixer, habitat camera, lizard brain.

## Docs to rewrite (peel ESPET voice)

Copy then strip product language (habitat, lizard, Nest camera, indexed-8 as law, 12 kHz as law).

From [`refs/guides/`](refs/guides/):

- **Move/rewrite:** `01-board-and-pins.md`, `02-soc-memory-smp.md`, `03-display-st7789.md`, `04-imu-qmi8658.md`, `06-touch-cst816.md`, `07-audio-es8311.md`, `08-power-battery.md`
- **Board bring-up only:** a new `09-bring-up.md` = silicon steps 1–3 + IMU whoami + (later) touch/audio/sleep. No Nest backdrop, no mesh raster, no cortex.
- **Leave in ESPET:** [`refs/guides/05-fusion-gravity-camera.md`](refs/guides/05-fusion-gravity-camera.md) (authored camera is product). Optionally add `boards/waveshare-touch-lcd-154/docs/guides/05-6axis-filter.md` as a generic complementary-filter note + Madgwick PDF from [`refs/SOURCES.md`](refs/SOURCES.md).

From [`refs/learn/`](refs/learn/): board literacy only, retitled so ESPET’s numbering can die:

- Move/rewrite: `02-how-chips-talk`, `06-first-pixels`, `08-imu-registers`, `13-power-and-boot` (as power/boot, not a course slot)
- **Do not move:** C-for-firmware-with-pet-examples, vectors/camera, quaternions-as-room-camera, indexed FB as habitat budget, gravity-locked cube, meshes, clips/springs, ESPET audio/touch/sleep product tests

Copy all `refs/**/*.pdf` and [`refs/SOURCES.md`](refs/SOURCES.md). Hardware-only slice of [`refs/glossary.md`](refs/glossary.md) and pin half of [`refs/learn/cheatsheet.md`](refs/learn/cheatsheet.md).

Code copy into the SKU folder: [`board-sim/`](board-sim/), [`firmware/main.c`](firmware/main.c), [`firmware/CMakeLists.txt`](firmware/CMakeLists.txt), [`sim.bat`](sim.bat), [`debug.bat`](debug.bat). Root [`.gitignore`](.gitignore) at the family repo. Rename window title / comments from “ESPET hello” to this board. Keep the sim/firmware split (firmware must not include SDL). Adjust CMake paths so `sim.bat` run from `boards/waveshare-touch-lcd-154/` still finds `firmware/main.c` next to `board-sim/`.

## How a new project starts (family README)

1. Clone `jpgma/esp32-s3`.
2. Pick a board folder (today: `boards/waveshare-touch-lcd-154`). Treat that `HARDWARE.md` as immutable unless the schematic proves a pin wrong.
3. Add `PRODUCT.md` in your *app* repo (FPS, pixel format, audio, radio). Do not put that in the SKU `HARDWARE.md`.
4. Copy or clone-from the SKU `firmware/` + `board-sim/` into the new project, or work in-tree until a second app exists.
5. Run `sim.bat` from the SKU folder until silicon; then `idf.py -C firmware build flash monitor` (still a placeholder CMake until IDF is wired — same as ESPET today).

Do not copy this SKU’s pin map onto a different ESP32-S3 module.

Touch/audio/power **examples are out of day one**; the guides still document them so a project does not invent pins.

## ESPET after the peel (link, no submodule)

ESPET keeps [`board-sim/`](board-sim/), [`firmware/`](firmware/), `sim.bat`, and all product architecture/learn.

**Remove from ESPET** (replace with a short pointer file, not a 404):

- Vendored PDFs under `refs/`
- Guides 01–04, 06–08 (and the hardware half of 09)
- Hardware learn lessons listed above
- Fat hardware glossary/cheatsheet duplication

**Keep, rewritten:**

- [`README.md`](README.md) — one line: hardware bible is `jpgma/esp32-s3` → `boards/waveshare-touch-lcd-154`
- [`architecture.md`](architecture.md) §0 / §2 — **thin** table: SKU link, 240×240 / no TE / no mag / `BAT_EN` / shared I2C / PSRAM/flash sizes. Full GPIO story lives in that board folder. Schematic link becomes GitHub URL (private: note that clones need access).
- [`refs/README.md`](refs/README.md) — “board docs moved; this folder is ESPET study only”
- [`refs/guides/05-fusion-gravity-camera.md`](refs/guides/05-fusion-gravity-camera.md), [`refs/guides/09-bring-up.md`](refs/guides/09-bring-up.md) — product checkpoints; PDF links point at `jpgma/esp32-s3`
- [`refs/learn/00-start-here.md`](refs/learn/00-start-here.md) — board literacy is the other repo; this course is the habitat
- Product learn 01, 03–05, 07, 09–12 stay; delete or stub 02/06/08/13–16 with links

`board-sim` will **diverge** (ESPET grows habitat firmware; the SKU folder stays a clean hello). That is intended. Align pins/`HARDWARE.md` only; do not share code until you want a HAL.

## GitHub

- Install/auth `gh` if missing; `gh repo create jpgma/esp32-s3 --private --source ... --remote origin --push`
- Do not use Cursor-hosted `origin repo create` (you asked for GitHub, and ESPET already lives at `github.com/jpgma/ESPET`)
- First commit on `jpgma/esp32-s3`; separate ESPET commit that drops the library (only when you ask to commit)

## Out of scope

- ESP-IDF `sdkconfig` / real flash path
- Touch, audio, sleep example firmware
- Git submodule
- Public PDF mirroring
- Renaming ESPET’s `board-sim` or changing `sim.bat` behavior
