#pragma once

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LEDC_LOW_SPEED_MODE = 0,
} ledc_mode_t;

typedef enum {
    LEDC_TIMER_10_BIT = 10,
} ledc_timer_bit_t;

typedef enum {
    LEDC_TIMER_0 = 0,
} ledc_timer_t;

typedef enum {
    LEDC_AUTO_CLK = 0,
} ledc_clk_cfg_t;

typedef enum {
    LEDC_CHANNEL_0 = 0,
} ledc_channel_t;

typedef enum {
    LEDC_FADE_NO_WAIT = 0,
} ledc_fade_mode_t;

typedef struct {
    ledc_mode_t speed_mode;
    ledc_timer_bit_t duty_resolution;
    ledc_timer_t timer_num;
    uint32_t freq_hz;
    ledc_clk_cfg_t clk_cfg;
} ledc_timer_config_t;

typedef struct {
    int gpio_num;
    ledc_mode_t speed_mode;
    ledc_channel_t channel;
    ledc_timer_t timer_sel;
    uint32_t duty;
    int hpoint;
} ledc_channel_config_t;

esp_err_t ledc_timer_config(const ledc_timer_config_t *timer_conf);
esp_err_t ledc_channel_config(const ledc_channel_config_t *ledc_conf);
esp_err_t ledc_fade_func_install(int intr_alloc_flags);
esp_err_t ledc_fade_stop(ledc_mode_t speed_mode, ledc_channel_t channel);
uint32_t ledc_get_duty(ledc_mode_t speed_mode, ledc_channel_t channel);
esp_err_t ledc_set_fade_with_time(ledc_mode_t speed_mode, ledc_channel_t channel, uint32_t target_duty, int desired_fade_time_ms);
esp_err_t ledc_fade_start(ledc_mode_t speed_mode, ledc_channel_t channel, ledc_fade_mode_t fade_mode);

#ifdef __cplusplus
}
#endif
