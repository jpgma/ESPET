# Desk

A host visit while the board is on USB. The corner card is where the pet can later sit, the way a desktop pet leaves its island for a corner of the screen. The glass keeps the current habitat room.

This is not a firmware room, not a door, and not the cortex. The v1 map stays Nest, Play, Hall, and Yard. Firmware does not know the window exists, the same rule as `board-sim`.

Play still works with the app closed. Unplug ends the visit. The lizard brain does not move onto the PC.

## This pass

`studio.bat` opens a small always-on-top card at the bottom-right of the work area, and a tray icon. The card holds the Espressif USB serial port (`VID_303A`, one `(COMx)`). It reads the log the firmware already prints. It does not reboot the chip: DTR and RTS stay off, and there is no esptool reset.

| Line | Shown as |
| :--- | :--- |
| `free internal` | heap internal, bytes. Printed once at boot, so it stays blank until that line arrives. |
| `ship` | latest ship, microseconds |
| `gap mean` | gap mean, microseconds |

The last few log lines sit under the numbers. Zero ports reads "no board". More than one port is reported and left closed. A busy port reads "in use".

The card owns the port. It cannot run beside `flash.bat --watch`. Release the port from the tray, then run `flash.bat`. Show on the tray menu takes the port back. Click the tray icon to hide the card, and click it again to show it. Close hides the card too. Quit is on the tray menu.

## Later

Not this pass:

- Draw the pet in the corner.
- Debug flags from the card (hitbox wireframe and the rest), drawn on the glass.
- The UDP cortex guest inside this same .NET process.
- Start the tray app at login, so the card appears when the cable is plugged in.

A periodic stats line in the firmware is not part of this. Those prints sit inside the frame-gap measurement.
