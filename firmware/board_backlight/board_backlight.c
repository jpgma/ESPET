#include "board_backlight.h"

#include "board_pins.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "bl";

/* Vendor backlight: LEDC 5 kHz, 10-bit, MOSFET on GPIO46.
 * A full swing is requested at 420 ms. At 5 kHz the fade hardware
 * holds each duty step for 2 cycles, so the glass takes about 410 ms. */
#define BL_FREQ_HZ 5000
#define BL_DUTY_MAX 1023u
#define BL_FADE_MS 420

enum {
    KEY_PWR = 0,
    KEY_PLUS = 1,
    KEY_BOOT = 2,
    KEY_COUNT = 3,
};

static const int s_pins[KEY_COUNT] = {PIN_PWR, PIN_PLUS, PIN_BOOT};
static int s_armed[KEY_COUNT];
static int s_down[KEY_COUNT];
static int s_stable[KEY_COUNT];
static int s_on = 1;
static board_key_fn s_on_key;

static void backlight_fade(int on)
{
    const uint32_t target = on ? BL_DUTY_MAX : 0;
    ESP_ERROR_CHECK(ledc_fade_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    const uint32_t cur = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    const uint32_t delta = cur > target ? cur - target : target - cur;
    if (delta == 0) {
        return;
    }
    /* Same duty-per-ms as a full swing, so a reverse mid-fade does not jump. */
    const int ms = (int)((delta * (uint32_t)BL_FADE_MS + BL_DUTY_MAX - 1) / BL_DUTY_MAX);
    ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, target, ms));
    ESP_ERROR_CHECK(ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT));
}

static void scan_keys(void)
{
    for (int i = 0; i < KEY_COUNT; i++) {
        int pressed = gpio_get_level(s_pins[i]) == 0;
        if (!s_armed[i]) {
            if (!pressed) {
                s_armed[i] = 1;
            }
            continue;
        }
        if (!pressed) {
            s_stable[i] = 0;
            s_down[i] = 0;
            continue;
        }
        if (s_stable[i] < 2) {
            s_stable[i]++;
        }
        if (s_stable[i] < 2 || s_down[i]) {
            continue;
        }
        s_down[i] = 1;
        if (i == KEY_PWR) {
            s_on = !s_on;
            backlight_fade(s_on);
            ESP_LOGI(TAG, "backlight %s", s_on ? "on" : "off");
        } else if (s_on_key) {
            s_on_key(i == KEY_PLUS ? BOARD_KEY_PLUS : BOARD_KEY_BOOT);
        }
    }
}

static void backlight_task(void *arg)
{
    (void)arg;
    for (;;) {
        scan_keys();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void board_backlight_on_press(board_key_fn fn)
{
    s_on_key = fn;
}

void board_backlight_init(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = BL_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    const ledc_channel_config_t ch = {
        .gpio_num = PIN_LCD_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    gpio_config_t keys = {
        .pin_bit_mask = (1ull << PIN_PWR) | (1ull << PIN_PLUS) | (1ull << PIN_BOOT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    ESP_ERROR_CHECK(ledc_channel_config(&ch));
    ESP_ERROR_CHECK(ledc_fade_func_install(0));
    backlight_fade(1);
    ESP_ERROR_CHECK(gpio_config(&keys));

    if (xTaskCreate(backlight_task, "bl", 3072, NULL, 1, NULL) != pdPASS) {
        ESP_LOGE(TAG, "backlight task failed");
    }
}
