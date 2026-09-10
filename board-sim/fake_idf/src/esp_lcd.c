#include "esp_lcd.h"

#include "board_sim.h"

#include <stddef.h>
#include <stdlib.h>

struct esp_lcd_panel_io_t {
    int dummy;
};

struct esp_lcd_panel_t {
    int inited;
    int on;
};

esp_err_t esp_lcd_new_panel_io_spi(spi_host_device_t host,
                                   const esp_lcd_panel_io_spi_config_t *io_config,
                                   esp_lcd_panel_io_handle_t *ret_io)
{
    (void)host;
    (void)io_config;
    if (!ret_io) {
        return ESP_ERR_INVALID_ARG;
    }
    *ret_io = calloc(1, sizeof(struct esp_lcd_panel_io_t));
    return *ret_io ? ESP_OK : ESP_FAIL;
}

esp_err_t esp_lcd_new_panel_st7789(esp_lcd_panel_io_handle_t io,
                                   const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel)
{
    (void)io;
    (void)panel_dev_config;
    if (!ret_panel) {
        return ESP_ERR_INVALID_ARG;
    }
    *ret_panel = calloc(1, sizeof(struct esp_lcd_panel_t));
    return *ret_panel ? ESP_OK : ESP_FAIL;
}

esp_err_t esp_lcd_panel_reset(esp_lcd_panel_handle_t panel)
{
    if (!panel) {
        return ESP_ERR_INVALID_ARG;
    }
    panel->inited = 0;
    return ESP_OK;
}

esp_err_t esp_lcd_panel_init(esp_lcd_panel_handle_t panel)
{
    if (!panel) {
        return ESP_ERR_INVALID_ARG;
    }
    panel->inited = 1;
    return ESP_OK;
}

esp_err_t esp_lcd_panel_draw_bitmap(esp_lcd_panel_handle_t panel, int x_start, int y_start,
                                    int x_end, int y_end, const void *color_data)
{
    if (!panel || !color_data) {
        return ESP_ERR_INVALID_ARG;
    }
    if (x_end <= x_start || y_end <= y_start) {
        return ESP_OK;
    }
    const size_t pixels = (size_t)(x_end - x_start) * (size_t)(y_end - y_start);
    board_sim_gram_blit(x_start, y_start, x_end, y_end, (const uint16_t *)color_data);
    board_sim_spi_delay_pixels(pixels);
    return ESP_OK;
}

esp_err_t esp_lcd_panel_invert_color(esp_lcd_panel_handle_t panel, bool invert)
{
    (void)panel;
    (void)invert;
    return ESP_OK;
}

esp_err_t esp_lcd_panel_mirror(esp_lcd_panel_handle_t panel, bool mirror_x, bool mirror_y)
{
    (void)panel;
    (void)mirror_x;
    (void)mirror_y;
    return ESP_OK;
}

esp_err_t esp_lcd_panel_swap_xy(esp_lcd_panel_handle_t panel, bool swap)
{
    (void)panel;
    (void)swap;
    return ESP_OK;
}

esp_err_t esp_lcd_panel_set_gap(esp_lcd_panel_handle_t panel, int x_gap, int y_gap)
{
    (void)panel;
    (void)x_gap;
    (void)y_gap;
    return ESP_OK;
}

esp_err_t esp_lcd_panel_disp_on_off(esp_lcd_panel_handle_t panel, bool on_off)
{
    if (!panel) {
        return ESP_ERR_INVALID_ARG;
    }
    panel->on = on_off ? 1 : 0;
    return ESP_OK;
}
