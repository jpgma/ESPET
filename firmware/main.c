/*
 * Hello firmware for the fake (and later real) Waveshare panel.
 * No SDL. Cube / pet logic does not belong here yet.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "hello";

#define LCD_H_RES 240
#define LCD_V_RES 240
#define PIN_LCD_CS 21
#define PIN_LCD_DC 45
#define PIN_LCD_RST 40
#define PIN_LCD_BL 46
#define PIN_I2C_SCL 41
#define PIN_I2C_SDA 42
#define QMI8658_ADDR 0x6B
#define QMI8658_WHO_AM_I 0x00
#define QMI8658_CTRL7 0x08
#define QMI8658_AX_L 0x35
#define QMI8658_GX_L 0x3B
#define ACCEL_LSB_PER_G 4096

static uint16_t s_fb[LCD_H_RES * LCD_V_RES];

static uint16_t rgb565(int r, int g, int b)
{
    if (r < 0) {
        r = 0;
    }
    if (r > 255) {
        r = 255;
    }
    if (g < 0) {
        g = 0;
    }
    if (g > 255) {
        g = 255;
    }
    if (b < 0) {
        b = 0;
    }
    if (b > 255) {
        b = 255;
    }
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static int16_t le16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void fill_from_imu(float ax, float ay, float az, float gyro_abs)
{
    int r = (int)(128.0f + ax * 90.0f);
    int g = (int)(128.0f + ay * 90.0f);
    int b = (int)(128.0f + az * 90.0f);
    uint16_t bg = rgb565(r, g, b);
    for (int i = 0; i < LCD_H_RES * LCD_V_RES; i++) {
        s_fb[i] = bg;
    }

    int bar = (int)(gyro_abs * 4.0f);
    if (bar > LCD_H_RES - 4) {
        bar = LCD_H_RES - 4;
    }
    if (bar < 0) {
        bar = 0;
    }
    uint16_t fg = rgb565(255, 255, 255);
    for (int y = 8; y < 20; y++) {
        for (int x = 2; x < 2 + bar; x++) {
            s_fb[y * LCD_H_RES + x] = fg;
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "hello Waveshare LCD + QMI8658");

    gpio_config_t bl = {
        .pin_bit_mask = 1ull << PIN_LCD_BL,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&bl) == 0 ? ESP_OK : ESP_FAIL);
    gpio_set_level(PIN_LCD_BL, 1);

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .flags.enable_internal_pullup = 1,
    };
    i2c_master_bus_handle_t bus = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    i2c_device_config_t imu_cfg = {
        .device_address = QMI8658_ADDR,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t imu = NULL;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &imu_cfg, &imu));

    uint8_t who_reg = QMI8658_WHO_AM_I;
    uint8_t who = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(imu, &who_reg, 1, &who, 1, 100));
    ESP_LOGI(TAG, "QMI8658 WHO_AM_I=0x%02x", who);
    if (who != 0x05) {
        ESP_LOGE(TAG, "unexpected WHO_AM_I");
    }

    uint8_t en[2] = {QMI8658_CTRL7, 0x03}; /* accel + gyro */
    ESP_ERROR_CHECK(i2c_master_transmit(imu, en, 2, 100));

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = PIN_LCD_CS,
        .dc_gpio_num = PIN_LCD_DC,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_cfg, &io));

    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    for (;;) {
        uint8_t ax_reg = QMI8658_AX_L;
        uint8_t raw[12];
        memset(raw, 0, sizeof(raw));
        ESP_ERROR_CHECK(i2c_master_transmit_receive(imu, &ax_reg, 1, raw, 12, 100));
        float ax = le16(&raw[0]) / (float)ACCEL_LSB_PER_G;
        float ay = le16(&raw[2]) / (float)ACCEL_LSB_PER_G;
        float az = le16(&raw[4]) / (float)ACCEL_LSB_PER_G;
        float gx = le16(&raw[6]) / 16.0f;
        float gy = le16(&raw[8]) / 16.0f;
        float gz = le16(&raw[10]) / 16.0f;
        float gyro_abs = gx >= 0 ? gx : -gx;
        gyro_abs += gy >= 0 ? gy : -gy;
        gyro_abs += gz >= 0 ? gz : -gz;

        fill_from_imu(ax, ay, az, gyro_abs);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, 0, LCD_H_RES, LCD_V_RES, s_fb));
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}
