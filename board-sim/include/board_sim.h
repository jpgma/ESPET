#pragma once

#include <stddef.h>
#include <stdint.h>

#define BOARD_SIM_LCD_W 240
#define BOARD_SIM_LCD_H 240
#define BOARD_SIM_SCALE 3
#define BOARD_SIM_QMI8658_WHO_AM_I 0x05

void board_sim_gram_init(void);
void board_sim_gram_blit(int x0, int y0, int x1, int y1, const uint16_t *rgb565);
void board_sim_gram_copy(uint16_t *dst);
void board_sim_spi_delay_pixels(size_t pixel_count);
void board_sim_set_spi_hz(int hz);
int board_sim_spi_hz(void);

void board_sim_imu_init(void);
void board_sim_imu_on_drag(int dx_px, int dy_px, float dt_s);
void board_sim_imu_tick(float dt_s);
int board_sim_imu_i2c_tx(const uint8_t *data, size_t len);
int board_sim_imu_i2c_txrx(const uint8_t *w, size_t wlen, uint8_t *r, size_t rlen);
