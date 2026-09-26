/*
 * Hello firmware for the fake (and later real) Waveshare panel.
 * No SDL. Cube / pet logic does not belong here yet.
 * This loop is the I2C owner: IMU poll + CST816 on INT. No ES7210.
 */

#include "board_pins.h"

#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#ifdef ESP_PLATFORM
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#else
#include "esp_lcd.h"
#endif
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "hello";

#define QMI8658_WHO_AM_I 0x00
#define QMI8658_CTRL1 0x02
#define QMI8658_CTRL2 0x03
#define QMI8658_CTRL3 0x04
#define QMI8658_CTRL7 0x08
#define QMI8658_AX_L 0x35
#define QMI8658_GX_L 0x3B
#define LCD_PCLK_HZ (80 * 1000 * 1000)

#define CST816_GESTURE 0x01
#define CST816_CHIP_ID 0xA7
#define CST816_MOTION_MASK 0xEC
#define CST816_IRQ_CTL 0xFA
#define CST816_DIS_AUTO_SLEEP 0xFE
#define CST816_EN_DCLICK 0x01

alignas(4) static uint16_t s_fb[LCD_H_RES * LCD_V_RES];
static volatile int s_touch_irq;
static int s_have_poke;
static float s_poke_u;
static float s_poke_v;

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

    if (!s_have_poke) {
        return;
    }
    int px = (int)(s_poke_u * (float)(LCD_H_RES - 1) + 0.5f);
    int py = (int)(s_poke_v * (float)(LCD_V_RES - 1) + 0.5f);
    if (px < 1) {
        px = 1;
    }
    if (px > LCD_H_RES - 2) {
        px = LCD_H_RES - 2;
    }
    if (py < 1) {
        py = 1;
    }
    if (py > LCD_V_RES - 2) {
        py = LCD_V_RES - 2;
    }
    uint16_t mark = rgb565(255, 220, 0);
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            s_fb[(py + dy) * LCD_H_RES + (px + dx)] = mark;
        }
    }
}

static void touch_isr(void *arg)
{
    (void)arg;
    s_touch_irq = 1;
}

static void drain_touch(i2c_master_dev_handle_t tp)
{
    uint8_t reg = CST816_GESTURE;
    uint8_t raw[6];
    memset(raw, 0, sizeof(raw));
    if (i2c_master_transmit_receive(tp, &reg, 1, raw, 6, 100) != ESP_OK) {
        return;
    }
    int x = ((raw[2] & 0x0F) << 8) | raw[3];
    int y = ((raw[4] & 0x0F) << 8) | raw[5];
    if (x < 0) {
        x = 0;
    }
    if (x > LCD_H_RES - 1) {
        x = LCD_H_RES - 1;
    }
    if (y < 0) {
        y = 0;
    }
    if (y > LCD_V_RES - 1) {
        y = LCD_V_RES - 1;
    }
    s_poke_u = (float)x / (float)(LCD_H_RES - 1);
    s_poke_v = (float)y / (float)(LCD_V_RES - 1);
    s_have_poke = 1;
    ESP_LOGI(TAG, "CST816 gesture=0x%02x finger=%u xy=%d,%d", raw[0], raw[1], x, y);
}

void app_main(void)
{
    ESP_LOGI(TAG, "hello Waveshare LCD + QMI8658 + CST816");

    gpio_config_t bat = {
        .pin_bit_mask = 1ull << PIN_BAT_EN,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&bat) == 0 ? ESP_OK : ESP_FAIL);
    gpio_set_level(PIN_BAT_EN, 1);

    gpio_config_t bl = {
        .pin_bit_mask = 1ull << PIN_LCD_BL,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&bl) == 0 ? ESP_OK : ESP_FAIL);
    gpio_set_level(PIN_LCD_BL, 1);

    gpio_config_t tp_rst = {
        .pin_bit_mask = 1ull << PIN_TOUCH_RST,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&tp_rst) == 0 ? ESP_OK : ESP_FAIL);
    gpio_set_level(PIN_TOUCH_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_TOUCH_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    gpio_config_t tp_int = {
        .pin_bit_mask = 1ull << PIN_TOUCH_INT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&tp_int) == 0 ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(gpio_set_intr_type(PIN_TOUCH_INT, GPIO_INTR_NEGEDGE) == 0 ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(gpio_install_isr_service(0) == 0 ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_TOUCH_INT, touch_isr, NULL) == 0 ? ESP_OK : ESP_FAIL);

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
#ifdef ESP_PLATFORM
        .clk_source = I2C_CLK_SRC_DEFAULT,
#endif
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

    i2c_device_config_t tp_cfg = {
        .device_address = CST816_ADDR,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t tp = NULL;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &tp_cfg, &tp));

    uint8_t who_reg = QMI8658_WHO_AM_I;
    uint8_t who = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(imu, &who_reg, 1, &who, 1, 100));
    ESP_LOGI(TAG, "QMI8658 WHO_AM_I=0x%02x", who);
    if (who != 0x05) {
        ESP_LOGE(TAG, "unexpected WHO_AM_I");
    }

    /* CTRL1 reset is 0x20: big-endian, no auto-increment. A 12-byte
     * read repeats AX_L until ADDR_AI is set. CTRL2 reset is ±2 g;
     * 4096 LSB/g is the ±8 g scale. Gyro /16 is ±2048 dps. */
    uint8_t cfg[][2] = {
        {QMI8658_CTRL1, 0x40}, /* ADDR_AI, little-endian */
        {QMI8658_CTRL2, 0x26}, /* ±8 g, 125 Hz */
        {QMI8658_CTRL3, 0x76}, /* ±2048 dps, ~112 Hz */
        {QMI8658_CTRL7, 0x03}, /* accel + gyro */
    };
    for (size_t i = 0; i < sizeof(cfg) / sizeof(cfg[0]); i++) {
        ESP_ERROR_CHECK(i2c_master_transmit(imu, cfg[i], 2, 100));
    }

    uint8_t chip_reg = CST816_CHIP_ID;
    uint8_t chip = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(tp, &chip_reg, 1, &chip, 1, 100));
    ESP_LOGI(TAG, "CST816 ChipID=0x%02x", chip);

    uint8_t motion[2] = {CST816_MOTION_MASK, CST816_EN_DCLICK};
    ESP_ERROR_CHECK(i2c_master_transmit(tp, motion, 2, 100));
    uint8_t irqctl[2] = {CST816_IRQ_CTL, 0x60};
    ESP_ERROR_CHECK(i2c_master_transmit(tp, irqctl, 2, 100));
    uint8_t nosleep[2] = {CST816_DIS_AUTO_SLEEP, 0x01};
    ESP_ERROR_CHECK(i2c_master_transmit(tp, nosleep, 2, 100));

#ifdef ESP_PLATFORM
    spi_bus_config_t spibus = {
        .mosi_io_num = PIN_LCD_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_LCD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * (int)sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &spibus, SPI_DMA_CH_AUTO));
#endif

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = PIN_LCD_CS,
        .dc_gpio_num = PIN_LCD_DC,
        .spi_mode = 3,
        .pclk_hz = LCD_PCLK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_cfg, &io));

    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
#ifdef ESP_PLATFORM
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
#endif
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    for (;;) {
        if (s_touch_irq) {
            s_touch_irq = 0;
            drain_touch(tp);
        }

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
