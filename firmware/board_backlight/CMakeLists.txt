idf_component_register(
    SRCS "board_backlight.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS ".."
    REQUIRES esp_driver_gpio esp_driver_ledc freertos log
)
