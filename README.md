# ESPET

Gravity-locked habitat of cube rooms on a [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54). The room is live flat triangles. The pet is one smooth-skinned mesh on six bones. Small rooms frame the pet; large rooms frame the house. Product rules: [architecture.md](architecture.md). Look refs: [refs/look/](refs/look/).

Hardware bible (pins, schematic, datasheets, hello + `board-sim`): **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). This repo keeps its own sim for habitat work; pin law lives there.

## Two pieces (do not mix them)

| Folder | What it is |
| :--- | :--- |
| [`firmware/`](firmware/) | The program that will run on the ESP32. The raster stub paints an indexed frame. Habitat meshes raster here. **No SDL, no mouse, no cube renderer in the simulator.** |
| [`board-sim/`](board-sim/) | A fake Waveshare: 240×240 ST7789 window, mouse-drag → QMI8658, **click → CST816** at 0x15. Your firmware does not know this exists. |

The rooms, springs, meshes, and pet are firmware work you add later. The simulator never draws a habitat and never rasterizes a cube.

## Daily loop (Windows)

You already have Visual Studio 18. `sim.bat` looks up an install that has the **C++ toolset** (`vcvarsall.bat`) — on this machine that is often **Build Tools** under `C:\Program Files (x86)\...`, even if the IDE lives on `F:`. From the repo root, in `cmd.exe`:

```text
sim.bat
```

That configures if needed, builds **Release**, and opens the panel. First run downloads SDL2 (needs network once). To step through firmware in the IDE (breakpoints in `firmware/main/main.c`):

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

SPI wire time follows the firmware `pclk_hz` (80 MHz). `BOARD_SIM_SPI_HZ` overrides it. The title shows fps, frame period, slack against 12.5 ms, and the SPI clock. CPU stretch toward 240 MHz is coarse. A sim overrun is not the silicon pass bar.

```text
set BOARD_SIM_SPI_HZ=40000000
sim.bat
```

The stub paints a steady indexed field. Keys **1**, **2**, and **3** are the Waveshare PWR, PLUS, and BOOT buttons. PLUS and BOOT step the palette. **Click** (little or no move) taps the fake CST816. This firmware does not read that tap yet. **Drag** past a few pixels tilts the fake IMU.

Raster tasks: [docs/raster](docs/raster/README.md).

## Real board

Not part of `sim.bat`. From the repo root, with the board on USB:

```text
flash.bat --watch
```

That builds `firmware/` with ESP-IDF, flashes the Espressif USB port, and opens an **ESPET** window with the serial log. `ESP_LOGI` lines show up there. Close that window to stop, or Ctrl+C in it. `flash.bat` without `--watch` flashes and returns. `--port COMx` overrides the port. ESP-IDF is `%USERPROFILE%\esp\esp-idf`, or `IDF_PATH` if that is already set. Do not put `board-sim/fake_idf` on that include path. Pins and `BAT_EN` are in the [hardware contract](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md).

## Mental model

`board-sim` is a pretend PCB (glass + IMU). `firmware` is the chip program.
