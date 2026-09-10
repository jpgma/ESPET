#include "board_sim.h"

#include <math.h>
#include <string.h>
#include <windows.h>

/* QMI8658-ish register map (Waveshare / QST). */
#define REG_WHO_AM_I 0x00
#define REG_REVISION 0x01
#define REG_CTRL1 0x02
#define REG_CTRL2 0x03
#define REG_CTRL3 0x04
#define REG_CTRL7 0x08
#define REG_STATUS0 0x2E
#define REG_AX_L 0x35
#define REG_GX_L 0x3B

#define CTRL7_AEN 0x01
#define CTRL7_GEN 0x02

#define ACCEL_LSB_PER_G 4096.0f /* ±8 g */
#define GYRO_LSB_PER_DPS 16.0f  /* ±2048 dps */

static CRITICAL_SECTION s_lock;
static uint8_t s_regs[256];
static uint8_t s_ptr;
static int s_have_ptr;

static float s_yaw;
static float s_pitch;
static float s_gyro_dps[3];
static float s_acc_g[3];

static void write_i16le(uint8_t *p, int16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static int16_t sat_i16(float v)
{
    if (v > 32767.0f) {
        return 32767;
    }
    if (v < -32768.0f) {
        return -32768;
    }
    return (int16_t)lroundf(v);
}

static void refresh_data_regs(void)
{
    /* World +Y up. Rest accel is R^T * (0, 1, 0) g. */
    const float cy = cosf(s_yaw);
    const float sy = sinf(s_yaw);
    const float cx = cosf(s_pitch);
    const float sx = sinf(s_pitch);
    /* R = Ry(yaw) * Rx(pitch); column 1 of R is device-Y in world. */
    s_acc_g[0] = sy * sx; /* device X */
    s_acc_g[1] = cx;      /* device Y */
    s_acc_g[2] = -cy * sx; /* device Z */

    memset(&s_regs[REG_AX_L], 0, 12);
    if (s_regs[REG_CTRL7] & CTRL7_AEN) {
        write_i16le(&s_regs[REG_AX_L], sat_i16(s_acc_g[0] * ACCEL_LSB_PER_G));
        write_i16le(&s_regs[REG_AX_L + 2], sat_i16(s_acc_g[1] * ACCEL_LSB_PER_G));
        write_i16le(&s_regs[REG_AX_L + 4], sat_i16(s_acc_g[2] * ACCEL_LSB_PER_G));
        s_regs[REG_STATUS0] |= 0x01;
    }
    if (s_regs[REG_CTRL7] & CTRL7_GEN) {
        write_i16le(&s_regs[REG_GX_L], sat_i16(s_gyro_dps[0] * GYRO_LSB_PER_DPS));
        write_i16le(&s_regs[REG_GX_L + 2], sat_i16(s_gyro_dps[1] * GYRO_LSB_PER_DPS));
        write_i16le(&s_regs[REG_GX_L + 4], sat_i16(s_gyro_dps[2] * GYRO_LSB_PER_DPS));
        s_regs[REG_STATUS0] |= 0x02;
    }
}

void board_sim_imu_init(void)
{
    InitializeCriticalSection(&s_lock);
    memset(s_regs, 0, sizeof(s_regs));
    s_regs[REG_WHO_AM_I] = BOARD_SIM_QMI8658_WHO_AM_I;
    s_regs[REG_REVISION] = 0x7C;
    s_regs[REG_CTRL2] = 0x04; /* ±8 g-ish */
    s_regs[REG_CTRL3] = 0x64; /* ±2048 dps-ish */
    s_ptr = 0;
    s_have_ptr = 0;
    s_yaw = 0.0f;
    s_pitch = 0.0f;
    s_gyro_dps[0] = s_gyro_dps[1] = s_gyro_dps[2] = 0.0f;
    refresh_data_regs();
}

void board_sim_imu_on_drag(int dx_px, int dy_px, float dt_s)
{
    const float rad_per_px = 0.008f;
    if (dt_s < 1e-4f) {
        dt_s = 1e-4f;
    }

    EnterCriticalSection(&s_lock);
    s_yaw += (float)dx_px * rad_per_px;
    s_pitch += (float)dy_px * rad_per_px;
    if (s_pitch > 1.45f) {
        s_pitch = 1.45f;
    }
    if (s_pitch < -1.45f) {
        s_pitch = -1.45f;
    }

    const float yaw_rate = ((float)dx_px * rad_per_px) / dt_s;
    const float pitch_rate = ((float)dy_px * rad_per_px) / dt_s;
    const float rad2deg = 57.2957795f;
    s_gyro_dps[0] = pitch_rate * rad2deg;
    s_gyro_dps[1] = yaw_rate * rad2deg;
    s_gyro_dps[2] = 0.0f;
    refresh_data_regs();
    LeaveCriticalSection(&s_lock);
}

void board_sim_imu_tick(float dt_s)
{
    (void)dt_s;
    /* No magnetometer: yaw holds. Gyro dies when you stop dragging. */
    EnterCriticalSection(&s_lock);
    s_gyro_dps[0] *= 0.5f;
    s_gyro_dps[1] *= 0.5f;
    s_gyro_dps[2] *= 0.5f;
    if (fabsf(s_gyro_dps[0]) < 0.05f) {
        s_gyro_dps[0] = 0.0f;
    }
    if (fabsf(s_gyro_dps[1]) < 0.05f) {
        s_gyro_dps[1] = 0.0f;
    }
    refresh_data_regs();
    LeaveCriticalSection(&s_lock);
}

static void apply_write(const uint8_t *data, size_t len)
{
    if (len == 0) {
        return;
    }
    s_ptr = data[0];
    s_have_ptr = 1;
    for (size_t i = 1; i < len; i++) {
        uint8_t addr = (uint8_t)(s_ptr + (uint8_t)(i - 1));
        if (addr == REG_WHO_AM_I || addr == REG_REVISION) {
            continue;
        }
        s_regs[addr] = data[i];
    }
    if (len > 1) {
        s_ptr = (uint8_t)(s_ptr + (len - 1));
    }
}

int board_sim_imu_i2c_tx(const uint8_t *data, size_t len)
{
    if (!data || len == 0) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    apply_write(data, len);
    refresh_data_regs();
    LeaveCriticalSection(&s_lock);
    return 0;
}

int board_sim_imu_i2c_txrx(const uint8_t *w, size_t wlen, uint8_t *r, size_t rlen)
{
    if (!r || rlen == 0) {
        return -1;
    }
    EnterCriticalSection(&s_lock);
    if (w && wlen > 0) {
        apply_write(w, wlen);
    }
    refresh_data_regs();
    uint8_t addr = s_have_ptr ? s_ptr : 0;
    for (size_t i = 0; i < rlen; i++) {
        r[i] = s_regs[(uint8_t)(addr + i)];
    }
    LeaveCriticalSection(&s_lock);
    return 0;
}
