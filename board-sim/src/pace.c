#include "board_sim.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <powrprof.h>

/* ProcessorInformation. Layout matches PROCESSOR_POWER_INFORMATION. */
typedef struct {
    ULONG number;
    ULONG max_mhz;
    ULONG current_mhz;
    ULONG mhz_limit;
    ULONG max_idle_state;
    ULONG current_idle_state;
} cpu_power_info_t;

#define STAMP_CAP 1024
#define FRAME_BUDGET_MS (1000.0 / 80.0)

static LARGE_INTEGER s_qpc_freq;
static LARGE_INTEGER s_last_yield;
static double s_extra_per_host; /* (host_MHz / target_MHz) - 1, else 0 */
static int s_host_mhz;
static int s_target_mhz;

static CRITICAL_SECTION s_stat_lock;
static int s_stat_ready;
static uint64_t s_stamps[STAMP_CAP];
static uint32_t s_stamp_head;
static uint32_t s_stamp_n;

static int read_host_mhz(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD n = si.dwNumberOfProcessors;
    if (n == 0) {
        n = 1;
    }
    if (n > 64) {
        n = 64;
    }
    cpu_power_info_t info[64];
    /* 11 = ProcessorInformation */
    LONG status = CallNtPowerInformation(11, NULL, 0, info, n * sizeof(info[0]));
    if (status < 0 || info[0].max_mhz == 0) {
        return 0;
    }
    return (int)info[0].max_mhz;
}

void board_sim_cpu_init(void)
{
    QueryPerformanceFrequency(&s_qpc_freq);
    QueryPerformanceCounter(&s_last_yield);
    InitializeCriticalSection(&s_stat_lock);
    s_stat_ready = 1;

    s_target_mhz = 240;
    s_extra_per_host = 0.0;
    const char *env = getenv("BOARD_SIM_CPU_HZ");
    if (env && env[0]) {
        int v = atoi(env);
        s_target_mhz = v > 0 ? v : 0;
    }

    s_host_mhz = read_host_mhz();
    if (s_target_mhz > 0 && s_host_mhz > s_target_mhz) {
        s_extra_per_host = ((double)s_host_mhz / (double)s_target_mhz) - 1.0;
        printf("board-sim: CPU stretch %d/%d MHz (coarse, not the chip)\n",
               s_host_mhz, s_target_mhz);
    } else if (s_target_mhz == 0) {
        printf("board-sim: CPU stretch off\n");
    } else if (s_host_mhz <= 0) {
        printf("board-sim: CPU stretch off (host MHz unknown)\n");
    } else {
        printf("board-sim: CPU stretch idle (%d MHz host, %d MHz target)\n",
               s_host_mhz, s_target_mhz);
    }
    fflush(stdout);
}

void board_sim_cpu_arm(void)
{
    QueryPerformanceCounter(&s_last_yield);
}

void board_sim_cpu_charge(void)
{
    if (s_extra_per_host <= 0.0 || s_qpc_freq.QuadPart <= 0) {
        QueryPerformanceCounter(&s_last_yield);
        return;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double dt_s = (double)(now.QuadPart - s_last_yield.QuadPart) / (double)s_qpc_freq.QuadPart;
    if (dt_s > 0.0) {
        double extra_s = dt_s * s_extra_per_host;
        if (extra_s >= 0.0005) {
            DWORD ms = (DWORD)(extra_s * 1000.0 + 0.5);
            if (ms > 0) {
                Sleep(ms);
            }
        }
    }
    QueryPerformanceCounter(&s_last_yield);
}

void board_sim_cpu_yield(void)
{
    QueryPerformanceCounter(&s_last_yield);
}

void board_sim_frame_commit(void)
{
    if (!s_stat_ready || s_qpc_freq.QuadPart <= 0) {
        return;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    EnterCriticalSection(&s_stat_lock);
    s_stamps[s_stamp_head] = (uint64_t)now.QuadPart;
    s_stamp_head = (s_stamp_head + 1) % STAMP_CAP;
    if (s_stamp_n < STAMP_CAP) {
        s_stamp_n++;
    }
    LeaveCriticalSection(&s_stat_lock);
}

void board_sim_frame_stats(board_sim_frame_stats_t *out)
{
    out->fps_valid = 0;
    out->fps = 0.0;
    out->period_valid = 0;
    out->period_ms = 0.0;
    out->slack_ms = 0.0;
    if (!s_stat_ready || s_qpc_freq.QuadPart <= 0 || s_stamp_n == 0) {
        return;
    }

    uint64_t stamps[STAMP_CAP];
    uint32_t n = 0;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    EnterCriticalSection(&s_stat_lock);
    n = s_stamp_n;
    uint32_t start = (s_stamp_head + STAMP_CAP - n) % STAMP_CAP;
    for (uint32_t i = 0; i < n; i++) {
        stamps[i] = s_stamps[(start + i) % STAMP_CAP];
    }
    LeaveCriticalSection(&s_stat_lock);

    const uint64_t window = (uint64_t)s_qpc_freq.QuadPart;
    const uint64_t now_q = (uint64_t)now.QuadPart;
    const uint64_t cutoff = now_q > window ? now_q - window : 0;
    int in_window = 0;
    int intervals = 0;
    double sum_ticks = 0.0;
    int have_prev = 0;
    uint64_t prev = 0;
    for (uint32_t i = 0; i < n; i++) {
        uint64_t t = stamps[i];
        if (t >= cutoff) {
            if (have_prev && t > prev) {
                sum_ticks += (double)(t - prev);
                intervals++;
            }
            in_window++;
        }
        prev = t;
        have_prev = 1;
    }
    if (in_window <= 0) {
        return;
    }
    out->fps_valid = 1;
    out->fps = (double)in_window;
    if (intervals > 0) {
        double period_s = (sum_ticks / (double)intervals) / (double)s_qpc_freq.QuadPart;
        out->period_valid = 1;
        out->period_ms = period_s * 1000.0;
        out->slack_ms = FRAME_BUDGET_MS - out->period_ms;
    }
}
