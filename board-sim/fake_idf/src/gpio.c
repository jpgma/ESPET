#include "driver/gpio.h"

int gpio_config(const gpio_config_t *cfg)
{
    (void)cfg;
    return 0;
}

int gpio_set_level(gpio_num_t gpio, uint32_t level)
{
    (void)gpio;
    (void)level;
    return 0;
}

int gpio_get_level(gpio_num_t gpio)
{
    (void)gpio;
    return 0;
}
