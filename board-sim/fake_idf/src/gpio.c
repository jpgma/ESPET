#include "driver/gpio.h"

#include <string.h>
#include <windows.h>

#define GPIO_SIM_N 64

static CRITICAL_SECTION s_lock;
static int s_lock_ready;
static int s_level[GPIO_SIM_N];
static gpio_int_type_t s_intr[GPIO_SIM_N];
static gpio_isr_t s_isr[GPIO_SIM_N];
static void *s_isr_arg[GPIO_SIM_N];
static int s_isr_service;

static void lock_init(void)
{
    if (!s_lock_ready) {
        InitializeCriticalSection(&s_lock);
        memset(s_level, 0, sizeof(s_level));
        /* Idle-high inputs that the glass uses as open-drain INT / RST. */
        s_level[47] = 1;
        s_level[48] = 1;
        s_lock_ready = 1;
    }
}

static int valid(gpio_num_t gpio)
{
    return gpio >= 0 && gpio < GPIO_SIM_N;
}

int gpio_config(const gpio_config_t *cfg)
{
    (void)cfg;
    lock_init();
    return 0;
}

int gpio_set_level(gpio_num_t gpio, uint32_t level)
{
    lock_init();
    if (!valid(gpio)) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    s_level[gpio] = level ? 1 : 0;
    LeaveCriticalSection(&s_lock);
    return 0;
}

int gpio_get_level(gpio_num_t gpio)
{
    int v = 0;
    lock_init();
    if (!valid(gpio)) {
        return 0;
    }
    EnterCriticalSection(&s_lock);
    v = s_level[gpio];
    LeaveCriticalSection(&s_lock);
    return v;
}

int gpio_set_intr_type(gpio_num_t gpio, gpio_int_type_t type)
{
    lock_init();
    if (!valid(gpio)) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    s_intr[gpio] = type;
    LeaveCriticalSection(&s_lock);
    return 0;
}

int gpio_install_isr_service(int intr_alloc_flags)
{
    (void)intr_alloc_flags;
    lock_init();
    s_isr_service = 1;
    return 0;
}

int gpio_isr_handler_add(gpio_num_t gpio, gpio_isr_t isr, void *arg)
{
    lock_init();
    if (!valid(gpio) || !isr) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    s_isr[gpio] = isr;
    s_isr_arg[gpio] = arg;
    LeaveCriticalSection(&s_lock);
    return 0;
}

int gpio_isr_handler_remove(gpio_num_t gpio)
{
    lock_init();
    if (!valid(gpio)) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    s_isr[gpio] = NULL;
    s_isr_arg[gpio] = NULL;
    LeaveCriticalSection(&s_lock);
    return 0;
}

void board_sim_gpio_irq_falling(gpio_num_t gpio)
{
    gpio_isr_t isr = NULL;
    void *arg = NULL;
    lock_init();
    if (!valid(gpio) || !s_isr_service) {
        return;
    }
    EnterCriticalSection(&s_lock);
    s_level[gpio] = 0;
    if (s_intr[gpio] == GPIO_INTR_NEGEDGE || s_intr[gpio] == GPIO_INTR_ANYEDGE) {
        isr = s_isr[gpio];
        arg = s_isr_arg[gpio];
    }
    LeaveCriticalSection(&s_lock);
    if (isr) {
        isr(arg);
    }
    EnterCriticalSection(&s_lock);
    s_level[gpio] = 1;
    LeaveCriticalSection(&s_lock);
}
