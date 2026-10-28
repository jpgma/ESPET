# ESPET

ESPET is a habitat of cube rooms. The rooms are locked to gravity. It runs on a [Waveshare ESP32-S3-Touch-LCD-1.54](https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.54). The device draws the room as live flat triangles. The pet is one smooth-skinned mesh with six bones. A small room shows the pet. A large room shows the full house. For the product rules, refer to [architecture.md](architecture.md). For the look references, refer to [refs/look/](refs/look/).

The hardware reference (pins, schematic, datasheets, hello program and `board-sim`) is in **[jpgma/esp32-s3](https://github.com/jpgma/esp32-s3)** → [`boards/waveshare-touch-lcd-154`](https://github.com/jpgma/esp32-s3/tree/main/boards/waveshare-touch-lcd-154). That repository is the authority for pins. This repository has its own simulator for the habitat work.

## Three parts (keep them separate)

| Folder | Description |
| :--- | :--- |
| [`firmware/`](firmware/) | The program for the ESP32. The raster stub paints an indexed frame. The habitat meshes are rasterized here. **The firmware has no SDL, no mouse and no cube renderer from the simulator.** |
| [`board-sim/`](board-sim/) | A simulated Waveshare board. It has a 240×240 ST7789 window. A mouse drag moves the QMI8658. **A click is a CST816 tap** at address 0x15. The firmware does not know about the simulator. |
| [`studio/`](studio/) | A desk card on the PC. It shows when the board is connected through USB. It is a visit on the host, not a room. The firmware does not know about it. |

You will add the rooms, springs, meshes and pet to the firmware later. The simulator does not draw a habitat. The simulator does not rasterize a cube.

## Daily procedure (Windows)

Visual Studio 18 must be installed. `sim.bat` finds an installation that has the **C++ toolset** (`vcvarsall.bat`). On this computer, that is usually **Build Tools** in `C:\Program Files (x86)\...`. This is also true when the IDE is on `F:`.

1. Open `cmd.exe` in the repository root.
2. Run this command:

```text
sim.bat
```

The script configures the build if necessary. It builds the **Release** configuration and opens the panel. The first run downloads SDL2. For this, a network connection is necessary one time.

To debug the firmware in the IDE (with breakpoints in `firmware/main/main.c`), do these steps:

1. Run this command:

```text
debug.bat
```

2. Visual Studio Community (the IDE on `F:`, not Build Tools) opens the generated solution. On VS 18, the solution is `build-sim\espet-board-sim.slnx`.
3. Select **Debug / x64**.
4. Push F5.

Do not run `sim.bat` at the same time.

| Command | Function |
| :--- | :--- |
| `sim.bat` | Configure (if necessary), build and run |
| `sim.bat --clean` | Delete `build-sim`, then configure, build and run |
| `sim.bat --debug` | Build Debug, not Release |
| `sim.bat --no-run` | Build only |
| `sim.bat --run-only` | Do not build. Start the last executable. |
| `sim.bat --help` | Show the flags |
| `debug.bat` | Open the simulator in Visual Studio for F5 debugging |

The SPI wire time uses the firmware `pclk_hz` (80 MHz). `BOARD_SIM_SPI_HZ` replaces this value. The title bar shows the fps, the frame period, the slack against 12.5 ms and the SPI clock. The simulator stretches the CPU time approximately to 240 MHz. If a frame is too long in the simulator, this does not mean that the silicon fails.

```text
set BOARD_SIM_SPI_HZ=40000000
sim.bat
```

The stub paints a steady indexed field. Keys **1**, **2** and **3** are the Waveshare PWR, PLUS and BOOT buttons. PLUS and BOOT change the palette. A **click** (with little or no movement) is a tap on the simulated CST816. This firmware does not read that tap yet. A **drag** of more than a few pixels tilts the simulated IMU.

For the raster tasks, refer to [docs/raster](docs/raster/README.md).

## Real board

This procedure is not part of `sim.bat`.

1. Connect the board through USB.
2. In the repository root, run this command:

```text
flash.bat --watch
```

The script builds `firmware/` with ESP-IDF. It flashes the Espressif USB port. Then it opens an **ESPET** window with the serial log. The `ESP_LOGI` lines show in that window.

- To stop, close the window, or push Ctrl+C in the window.
- `flash.bat` without `--watch` flashes the board and then stops.
- `--port COMx` sets a different port.

ESP-IDF is in `%USERPROFILE%\esp\esp-idf`. If `IDF_PATH` is set, the script uses `IDF_PATH`. Do not put `board-sim/fake_idf` on that include path. The pins and `BAT_EN` are in the [hardware contract](https://github.com/jpgma/esp32-s3/blob/main/boards/waveshare-touch-lcd-154/HARDWARE.md).

## Desk

In the repository root, run this command:

```text
studio.bat
```

This command opens the corner card and the tray icon. The card holds the Espressif COM port. It shows the serial log (heap, ship and gap mean). Before you run `flash.bat`, release the port from the tray. Do not run the card at the same time as `flash.bat --watch`. For more data, refer to [docs/studio.md](docs/studio.md).

## Mental model

`board-sim` is a simulated PCB (glass and IMU). `firmware` is the program for the chip. `studio` is the desk when the cable is connected.
