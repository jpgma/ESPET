idf_component_register(
    SRCS "main.c" "esp32/platform.c"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "esp32" ".."
    REQUIRES esp_lcd esp_driver_gpio esp_driver_spi board_backlight
    PRIV_REQUIRES nvs_flash esp_event esp_netif esp_wifi esp_timer esp_rom heap esp_hw_support
)
