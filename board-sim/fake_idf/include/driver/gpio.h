#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int gpio_num_t;
typedef void (*gpio_isr_t)(void *arg);

#define GPIO_NUM_NC -1
#define GPIO_NUM_0 0
#define GPIO_NUM_1 1
#define GPIO_NUM_2 2
#define GPIO_NUM_3 3
#define GPIO_NUM_4 4
#define GPIO_NUM_5 5
#define GPIO_NUM_6 6
#define GPIO_NUM_7 7
#define GPIO_NUM_21 21
#define GPIO_NUM_38 38
#define GPIO_NUM_39 39
#define GPIO_NUM_40 40
#define GPIO_NUM_41 41
#define GPIO_NUM_42 42
#define GPIO_NUM_45 45
#define GPIO_NUM_46 46
#define GPIO_NUM_47 47
#define GPIO_NUM_48 48

typedef enum {
    GPIO_MODE_DISABLE = 0,
    GPIO_MODE_INPUT = 1,
    GPIO_MODE_OUTPUT = 2,
} gpio_mode_t;

typedef enum {
    GPIO_INTR_DISABLE = 0,
    GPIO_INTR_POSEDGE = 1,
    GPIO_INTR_NEGEDGE = 2,
    GPIO_INTR_ANYEDGE = 3,
} gpio_int_type_t;

typedef struct {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    int pull_up_en;
    int pull_down_en;
    int intr_type;
} gpio_config_t;

int gpio_config(const gpio_config_t *cfg);
int gpio_set_level(gpio_num_t gpio, uint32_t level);
int gpio_get_level(gpio_num_t gpio);
int gpio_set_intr_type(gpio_num_t gpio, gpio_int_type_t type);
int gpio_install_isr_service(int intr_alloc_flags);
int gpio_isr_handler_add(gpio_num_t gpio, gpio_isr_t isr, void *arg);
int gpio_isr_handler_remove(gpio_num_t gpio);

/* Sim-only: falling edge used for CST816 INT. */
void board_sim_gpio_irq_falling(gpio_num_t gpio);

#ifdef __cplusplus
}
#endif
