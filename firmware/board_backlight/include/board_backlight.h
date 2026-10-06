#pragma once

/* GPIO46 backlight fades on at start. PWR (GPIO5) fades it off or back on.
 * PLUS (GPIO4) and BOOT (GPIO0) are reported as presses. All three are active low. */

typedef enum {
    BOARD_KEY_PLUS = 0,
    BOARD_KEY_BOOT = 1,
} board_key_t;

typedef void (*board_key_fn)(board_key_t key);

void board_backlight_on_press(board_key_fn fn);
void board_backlight_init(void);
