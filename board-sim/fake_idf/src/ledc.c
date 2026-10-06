#include "driver/ledc.h"

/* Duty only. The sim does not draw the backlight. */

static uint32_t s_duty;

esp_err_t ledc_timer_config(const ledc_timer_config_t *timer_conf)
{
    (void)timer_conf;
    return ESP_OK;
}

esp_err_t ledc_channel_config(const ledc_channel_config_t *ledc_conf)
{
    if (!ledc_conf) {
        return ESP_ERR_INVALID_ARG;
    }
    s_duty = ledc_conf->duty;
    return ESP_OK;
}

esp_err_t ledc_fade_func_install(int intr_alloc_flags)
{
    (void)intr_alloc_flags;
    return ESP_OK;
}

esp_err_t ledc_fade_stop(ledc_mode_t speed_mode, ledc_channel_t channel)
{
    (void)speed_mode;
    (void)channel;
    return ESP_OK;
}

uint32_t ledc_get_duty(ledc_mode_t speed_mode, ledc_channel_t channel)
{
    (void)speed_mode;
    (void)channel;
    return s_duty;
}

esp_err_t ledc_set_fade_with_time(ledc_mode_t speed_mode, ledc_channel_t channel, uint32_t target_duty, int desired_fade_time_ms)
{
    (void)speed_mode;
    (void)channel;
    (void)desired_fade_time_ms;
    s_duty = target_duty;
    return ESP_OK;
}

esp_err_t ledc_fade_start(ledc_mode_t speed_mode, ledc_channel_t channel, ledc_fade_mode_t fade_mode)
{
    (void)speed_mode;
    (void)channel;
    (void)fade_mode;
    return ESP_OK;
}
