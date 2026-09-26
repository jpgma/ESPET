#pragma once

/*
 * Waveshare ESP32-S3-Touch-LCD-1.54 — product pin / I2C law.
 * Full table: jpgma/esp32-s3 boards/waveshare-touch-lcd-154/HARDWARE.md
 * Do not invent GPIOs. GPIO18 is TF D1, not a spare.
 */

#define LCD_H_RES 240
#define LCD_V_RES 240

#define PIN_BAT_EN 2
#define PIN_VBAT_ADC 1
#define PIN_CHG_STAT 3
#define PIN_PWR 5
#define PIN_PLUS 4
#define PIN_BOOT 0

#define PIN_LCD_CS 21
#define PIN_LCD_CLK 38
#define PIN_LCD_MOSI 39
#define PIN_LCD_DC 45
#define PIN_LCD_RST 40
#define PIN_LCD_BL 46

#define PIN_I2C_SCL 41
#define PIN_I2C_SDA 42

#define PIN_IMU_INT 6
#define PIN_TOUCH_INT 48
#define PIN_TOUCH_RST 47

#define PIN_PA 7
#define PIN_I2S_MCLK 8
#define PIN_I2S_BCLK 9
#define PIN_I2S_WS 10
#define PIN_I2S_DIN 11  /* ES7210 SDOUT — unused */
#define PIN_I2S_DOUT 12 /* ES8311 DSDIN */

#define PIN_USB_DM 19
#define PIN_USB_DP 20

/* TF unused: CLK 16, CMD 15, D0 17, D1 18, D2 13, D3 14. */

#define QMI8658_ADDR 0x6B
#define CST816_ADDR 0x15
#define ES8311_ADDR 0x18
#define ES7210_ADDR 0x40 /* never probe */

#define ACCEL_LSB_PER_G 4096 /* ±8 g until CTRL2 changes */
