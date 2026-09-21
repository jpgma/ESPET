# ESPET

Gravity-locked habitat of cube rooms on a [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54). Small rooms are the close-up pet; large rooms are the house — a floor shadow, plants you walk behind, a bowl you munch at. The room is a photograph; the pet, toys, and knockables are live low-poly meshes. Product rules: [architecture.md](architecture.md).

Hardware bible (pins, schematic, datasheets, hello + `board-sim`): **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). This repo keeps its own sim for habitat work; pin law lives there.

## Two pieces (do not mix them)

| Folder | What it is |
| :--- | :--- |
| [`firmware/`](firmware/) | The program that will run on the ESP32. Talks to the LCD (`esp_lcd`) and IMU (I2C QMI8658). Habitat meshes raster here. **No SDL, no mouse, no cube renderer in the simulator.** |
| [`board-sim/`](board-sim/) | A fake Waveshare: 240×240 ST7789 window + mouse-drag feeding QMI8658-like accel/gyro. Your firmware does not know this exists. |

The rooms, springs, meshes, and pet are firmware work you add later. The simulator never draws a habitat and never rasterizes a cube.

## Daily loop (Windows)

You already have Visual Studio 18. `sim.bat` looks up an install that has the **C++ toolset** (`vcvarsall.bat`) — on this machine that is often **Build Tools** under `C:\Program Files (x86)\...`, even if the IDE lives on `F:`. From the repo root, in `cmd.exe`:

```text
sim.bat
```

That configures if needed, builds **Release**, and opens the panel. First run downloads SDL2 (needs network once). To step through firmware in the IDE (breakpoints in `firmware/main.c`):

```text
debug.bat
```

That opens the generated solution (`build-sim\espet-board-sim.slnx` on VS 18) in Visual Studio Community (the IDE on `F:`, not Build Tools). Pick **Debug / x64** and press F5. Do not run `sim.bat` at the same time.

| Command | Meaning |
| :--- | :--- |
| `sim.bat` | configure (if needed) + build + run |
| `sim.bat --clean` | wipe `build-sim`, then the same |
| `sim.bat --debug` | Debug instead of Release |
| `sim.bat --no-run` | build only |
| `sim.bat --run-only` | skip build; launch last exe |
| `sim.bat --help` | flags |
| `debug.bat` | open the sim in Visual Studio for F5 debugging |

Optional fake SPI cost (off by default):

```text
set BOARD_SIM_SPI_HZ=40000000
sim.bat
```

Drag in the window to tilt the fake IMU. The hello firmware tints the screen from accel and draws a white bar from gyro rate so you can see both axes.

## Real board (later)

Not part of `sim.bat`. Turn `firmware/` into an ESP-IDF project and:

```text
idf.py -C firmware build flash monitor
```

Same `firmware/main.c` (plus real `sdkconfig` pins). Do not put `board-sim/fake_idf` on that include path. Pins and `BAT_EN` are in the [hardware contract](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md).

## Mental model

`board-sim` is a pretend PCB (glass + IMU). `firmware` is the chip program.
