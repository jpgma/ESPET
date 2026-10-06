/*
 * Raster firmware. Plans: docs/raster/README.md
 *
 * sim.bat
 * On the board, from the firmware folder: idf.py build flash monitor
 * This stub paints a steady marker. It presents as fast as the 80 MHz
 * wire allows. The glass cap is 80 Hz (docs/raster). A full frame is
 * already about that cap, so this loop does not sleep. Task 3 replaces
 * the loop.
 */

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"

#include "board_backlight.h"
#include "board_pins.h"
#include "palette.h"
#include "platform.h"

static const char *TAG = "raster";

#define FRAME_BUFFER_SIZE_BYTES (LCD_H_RES * LCD_V_RES)
#define PALETTE_SIZE 256
#define BAND_ROWS 8
#define ROW_MASK_SIZE_BYTES (LCD_V_RES / 8)

#define GLASS_PERIOD_US 12500LL

static uint8_t s_indexed_framebuffer[FRAME_BUFFER_SIZE_BYTES];
/* 0-63 actor ramps, then Nest. Hall, Yard, Play, and night stay in palette.h. */
static uint16_t s_pallete[PALETTE_SIZE] = {
    ESPET_ACTOR_PALETTE_RGB565,
    ESPET_ROOM_NEST_RGB565,
};
_Static_assert(sizeof((uint16_t[]){ESPET_ACTOR_PALETTE_RGB565}) / sizeof(uint16_t) == 64,
               "actor palette is indices 0-63");
_Static_assert(sizeof((uint16_t[]){ESPET_ROOM_NEST_RGB565}) / sizeof(uint16_t) <= PALETTE_SIZE - 64,
               "nest palette fits indices 64-255");
DMA_ATTR static uint16_t s_bands[2][BAND_ROWS * LCD_H_RES];
DMA_ATTR static uint8_t s_row_mask[ROW_MASK_SIZE_BYTES];

static volatile int s_bg;

static void on_key(board_key_t key)
{
    if (key == BOARD_KEY_PLUS) {
        s_bg = (s_bg + 1) % PALETTE_SIZE;
    } else {
        s_bg = (s_bg + PALETTE_SIZE - 1) % PALETTE_SIZE;
    }
    ESP_LOGI(TAG, "background %d", s_bg);
}

void app_main(void)
{
    gpio_config_t bat = {
        .pin_bit_mask = 1ull << PIN_BAT_EN,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&bat));
    gpio_set_level(PIN_BAT_EN, 1);
    board_backlight_on_press(on_key);
    board_backlight_init();

    ESP_LOGI(TAG, "raster stub (docs/raster)");
    platform_prepare(s_indexed_framebuffer, sizeof(s_indexed_framebuffer),
                     s_pallete, sizeof(s_pallete),
                     s_bands, sizeof(s_bands));

    spi_bus_config_t buscfg = {
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
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = PIN_LCD_CS,
        .dc_gpio_num = PIN_LCD_DC,
        .spi_mode = 3,
        .pclk_hz = 80 * 1000 * 1000,
        .trans_queue_depth = 1,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_cfg, &io));

    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    platform_spi_arm(io);

    for (;;) {
        platform_pace(GLASS_PERIOD_US);

        int slot = 0;
        for (int y1 = 0; y1 < LCD_V_RES; y1 += BAND_ROWS) {
            uint16_t *bounce = s_bands[slot];
            for (int row = 0; row < BAND_ROWS; row++) {
                for (int x = 0; x < LCD_H_RES; x++) {
                    bounce[row * LCD_H_RES + x] = s_pallete[s_bg];
                }
            }
            /* Expand above overlapped the previous band. Draw only after it finishes. */
            platform_wait_dma();
            ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(
                panel, 0, y1, LCD_H_RES, y1 + BAND_ROWS, bounce));
            platform_dma_queued();
            slot ^= 1;
        }
        platform_wait_dma();
    }
}
