# Budget

Arithmetic for this rasterizer. Silicon measurements stay blank until that task logs them.

## Memory

| Line | Arithmetic | Result |
| :--- | :--- | ---: |
| Indexed frame | 240 × 240 × 1 byte | 57600 bytes |
| Palette | 256 × 2 bytes | 512 bytes |
| Bounce | 2 × 8 × 240 × 2 bytes | 7680 bytes |
| Row mask | 240 / 8 | 30 bytes |
| Triangle scratch | 1024 × (2×3 + 1 + 2) bytes | 9216 bytes |
| Pose mailbox | 3 × (4 + 2 + 2 + 1 + 1 + (12 + 16) × 8) bytes | 696 bytes |

## Time

| Line | Arithmetic | Result |
| :--- | :--- | ---: |
| Full-frame SPI | 240 × 240 × 16 / 80000000 | 11520 µs |
| Glass cap | 1000000 / 80 | 12500 µs |
| Margin | 12500 − 11520 | 980 µs |
| Core 1 cycles | 240000000 / 80 | 3000000 |

## Measured

| Line | When | Result |
| :--- | :--- | :--- |
| Free internal heap | Wi-Fi started, Bluetooth off | 184387 bytes |
| SPI on the glass | Task 8 | |
| Raster, default scene | Task 7 | |
| Raster, full-frame animation | Task 8 | |
