#pragma once

#include <stddef.h>
#include <stdint.h>

#define BOARD_SIM_LCD_W 240
#define BOARD_SIM_LCD_H 240
#define BOARD_SIM_SCALE 3
#define BOARD_SIM_QMI8658_WHO_AM_I 0x05
#define BOARD_SIM_CST816_CHIP_ID 0xB5 /* placeholder; silicon ChipID at 0xA7 is law */

void board_sim_gram_init(void);
/* 1 when this blit finishes covering every row since the previous frame. */
int board_sim_gram_blit(int x0, int y0, int x1, int y1, const uint16_t *rgb565);
void board_sim_gram_copy(uint16_t *dst);
void board_sim_spi_delay_pixels(size_t pixel_count);
/* Env override. Locks out the firmware pclk. */
void board_sim_set_spi_hz(int hz);
/* Firmware pclk. Ignored after board_sim_set_spi_hz. */
void board_sim_adopt_spi_hz(int hz);
int board_sim_spi_hz(void);

/* Coarse host stretch toward BOARD_SIM_CPU_HZ (default 240 MHz). Not the chip. */
void board_sim_cpu_init(void);
void board_sim_cpu_arm(void);
void board_sim_cpu_charge(void);
void board_sim_cpu_yield(void);

void board_sim_frame_commit(void);

typedef struct {
    int fps_valid;
    double fps;
    int period_valid;
    double period_ms;
    double slack_ms;
} board_sim_frame_stats_t;

void board_sim_frame_stats(board_sim_frame_stats_t *out);

void board_sim_imu_init(void);
void board_sim_imu_on_drag(int dx_px, int dy_px, float dt_s);
void board_sim_imu_tick(float dt_s);
int board_sim_imu_i2c_tx(const uint8_t *data, size_t len);
int board_sim_imu_i2c_txrx(const uint8_t *w, size_t wlen, uint8_t *r, size_t rlen);

void board_sim_touch_init(void);
void board_sim_touch_on_tap(int x, int y, int double_click);
int board_sim_touch_i2c_tx(const uint8_t *data, size_t len);
int board_sim_touch_i2c_txrx(const uint8_t *w, size_t wlen, uint8_t *r, size_t rlen);
