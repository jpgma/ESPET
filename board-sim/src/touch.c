#include "board_sim.h"

#include "driver/gpio.h"

#include <string.h>
#include <windows.h>

#define REG_GESTURE 0x01
#define REG_FINGER 0x02
#define REG_XH 0x03
#define REG_XL 0x04
#define REG_YH 0x05
#define REG_YL 0x06
#define REG_CHIP_ID 0xA7
#define REG_MOTION_MASK 0xEC
#define REG_IRQ_CTL 0xFA
#define REG_DIS_AUTO_SLEEP 0xFE

#define GESTURE_CLICK 0x05
#define GESTURE_DOUBLE 0x0B

#define PIN_TOUCH_RST 47
#define PIN_TOUCH_INT 48

static CRITICAL_SECTION s_lock;
static uint8_t s_regs[256];
static uint8_t s_ptr;
static int s_have_ptr;

static void apply_write(const uint8_t *data, size_t len)
{
    if (len == 0) {
        return;
    }
    s_ptr = data[0];
    s_have_ptr = 1;
    for (size_t i = 1; i < len; i++) {
        uint8_t addr = (uint8_t)(s_ptr + (uint8_t)(i - 1));
        if (addr == REG_CHIP_ID) {
            continue;
        }
        s_regs[addr] = data[i];
    }
    if (len > 1) {
        s_ptr = (uint8_t)(s_ptr + (len - 1));
    }
}

static int rst_held(void)
{
    return gpio_get_level(PIN_TOUCH_RST) == 0;
}

void board_sim_touch_init(void)
{
    InitializeCriticalSection(&s_lock);
    memset(s_regs, 0, sizeof(s_regs));
    s_regs[REG_CHIP_ID] = BOARD_SIM_CST816_CHIP_ID;
    s_ptr = 0;
    s_have_ptr = 0;
}

void board_sim_touch_on_tap(int x, int y, int double_click)
{
    if (x < 0) {
        x = 0;
    }
    if (x > 239) {
        x = 239;
    }
    if (y < 0) {
        y = 0;
    }
    if (y > 239) {
        y = 239;
    }

    EnterCriticalSection(&s_lock);
    s_regs[REG_GESTURE] = (uint8_t)(double_click ? GESTURE_DOUBLE : GESTURE_CLICK);
    s_regs[REG_FINGER] = 1;
    s_regs[REG_XH] = (uint8_t)((x >> 8) & 0x0F);
    s_regs[REG_XL] = (uint8_t)(x & 0xFF);
    s_regs[REG_YH] = (uint8_t)((y >> 8) & 0x0F);
    s_regs[REG_YL] = (uint8_t)(y & 0xFF);
    LeaveCriticalSection(&s_lock);

    if (!rst_held()) {
        board_sim_gpio_irq_falling(PIN_TOUCH_INT);
    }
}

int board_sim_touch_i2c_tx(const uint8_t *data, size_t len)
{
    if (rst_held()) {
        return -1;
    }
    if (!data || len == 0) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    apply_write(data, len);
    LeaveCriticalSection(&s_lock);
    return 0;
}

int board_sim_touch_i2c_txrx(const uint8_t *w, size_t wlen, uint8_t *r, size_t rlen)
{
    if (rst_held()) {
        return -1;
    }
    if (!r || rlen == 0) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    if (w && wlen > 0) {
        apply_write(w, wlen);
    }
    uint8_t addr = s_have_ptr ? s_ptr : 0;
    for (size_t i = 0; i < rlen; i++) {
        r[i] = s_regs[(uint8_t)(addr + i)];
    }
    LeaveCriticalSection(&s_lock);
    return 0;
}
