# Desk

The desk is a visit on the host. It is available only when the board is connected through USB. Later, the pet can sit on the corner card. This is similar to a desktop pet that goes from its island to a corner of the screen. The glass continues to show the current habitat room.

The desk is not a firmware room. It is not a door and it is not the cortex. The v1 map does not change: Nest, Play, Hall and Yard. The firmware does not know about the window. This is the same rule as for `board-sim`.

The pet continues to operate when the app is closed. When you disconnect the cable, the visit stops. The lizard brain does not move to the PC.

## This pass

`studio.bat` opens a small card and a tray icon. The card is always on top, at the bottom-right of the work area. The card holds the Espressif USB serial port (`VID_303A`, one `(COMx)`). It reads the log that the firmware prints. It does not reset the chip: DTR and RTS stay off, and the card does not use an esptool reset.

| Line | Shown as |
| :--- | :--- |
| `free internal` | Internal heap, in bytes. The firmware prints this line one time at boot. The field stays empty until the line arrives. |
| `ship` | Latest ship time, in microseconds |
| `gap mean` | Mean gap, in microseconds |

The last log lines show below the numbers. If there is no port, the card shows "no board". If there is more than one port, the card shows this condition and does not open a port. If the port is busy, the card shows "in use".

The card owns the port. You cannot use the card and `flash.bat --watch` at the same time. To flash the board, do these steps:

1. Release the port from the tray.
2. Run `flash.bat`.
3. To get the port again, select **Show** on the tray menu.

Other controls:

- To hide the card, click the tray icon. To show the card again, click the icon again.
- **Close** also hides the card.
- **Quit** is on the tray menu.

## Later

These items are not in this pass:

- Draw the pet in the corner.
- Set debug flags from the card (for example, the hitbox wireframe). The glass shows them.
- Put the UDP cortex guest in this .NET process.
- Start the tray app at login. Then the card shows when you connect the cable.

A periodic statistics line in the firmware is not part of this work. Those prints are in the frame-gap measurement.
