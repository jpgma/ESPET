#define SDL_MAIN_HANDLED

#include "board_sim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

void app_main(void);

static uint16_t s_present[BOARD_SIM_LCD_W * BOARD_SIM_LCD_H];

/* Two 20 ms backlight scans, plus a frame, so a tap still lands. */
#define BUTTON_HOLD_MS 80

static int s_button_held[3];
static Uint32 s_button_until[3];

static int button_from_scancode(SDL_Scancode code)
{
    switch (code) {
    case SDL_SCANCODE_1:
    case SDL_SCANCODE_KP_1:
        return 0;
    case SDL_SCANCODE_2:
    case SDL_SCANCODE_KP_2:
        return 1;
    case SDL_SCANCODE_3:
    case SDL_SCANCODE_KP_3:
        return 2;
    default:
        return -1;
    }
}

static void buttons_apply(Uint32 now)
{
    for (int i = 0; i < 3; i++) {
        int down = s_button_held[i] || (s_button_until[i] != 0 && now < s_button_until[i]);
        board_sim_button(i, down);
        if (!down) {
            s_button_until[i] = 0;
        }
    }
}

static void buttons_release_all(void)
{
    for (int i = 0; i < 3; i++) {
        s_button_held[i] = 0;
        s_button_until[i] = 0;
        board_sim_button(i, 0);
    }
}

static void format_spi(char *dst, size_t n, int hz)
{
    if (hz <= 0) {
        snprintf(dst, n, "—");
    } else if (hz % 1000000 == 0) {
        snprintf(dst, n, "%d MHz", hz / 1000000);
    } else {
        snprintf(dst, n, "%d Hz", hz);
    }
}

static void format_title(char *dst, size_t n)
{
    board_sim_frame_stats_t st;
    board_sim_frame_stats(&st);
    char spi[32];
    format_spi(spi, sizeof(spi), board_sim_spi_hz());
    if (!st.fps_valid) {
        snprintf(dst, n, "espet  raster  0 fps  —  slack —  spi %s", spi);
        return;
    }
    if (!st.period_valid) {
        snprintf(dst, n, "espet  raster  %.0f fps  —  slack —  spi %s", st.fps, spi);
        return;
    }
    snprintf(dst, n, "espet  raster  %.0f fps  %.0f ms  slack %+.0f ms  spi %s",
             st.fps, st.period_ms, st.slack_ms, spi);
}

static int firmware_thread(void *unused)
{
    (void)unused;
    board_sim_cpu_arm();
    app_main();
    return 0;
}

static void rgb565_to_rgba8888(const uint16_t *src, uint32_t *dst, int n)
{
    for (int i = 0; i < n; i++) {
        uint16_t p = src[i];
        uint32_t r = ((p >> 11) & 0x1F) << 3;
        uint32_t g = ((p >> 5) & 0x3F) << 2;
        uint32_t b = (p & 0x1F) << 3;
        r |= r >> 5;
        g |= g >> 6;
        b |= b >> 5;
        dst[i] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    const char *spi = getenv("BOARD_SIM_SPI_HZ");
    board_sim_gram_init();
    board_sim_cpu_init();
    board_sim_imu_init();
    board_sim_touch_init();
    if (spi && spi[0]) {
        board_sim_set_spi_hz(atoi(spi));
        printf("board-sim: SPI override %d Hz\n", board_sim_spi_hz());
    }

    SDL_SetMainReady();
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    const int win_w = BOARD_SIM_LCD_W * BOARD_SIM_SCALE;
    const int win_h = BOARD_SIM_LCD_H * BOARD_SIM_SCALE;
    char title[160];
    format_title(title, sizeof(title));
    SDL_Window *win = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h, SDL_WINDOW_SHOWN);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        ren = SDL_CreateRenderer(win, -1, 0);
    }
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(ren, BOARD_SIM_LCD_W, BOARD_SIM_LCD_H);
    SDL_RenderSetIntegerScale(ren, SDL_TRUE);

    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         BOARD_SIM_LCD_W, BOARD_SIM_LCD_H);
    if (!tex) {
        fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);

    SDL_Thread *fw = SDL_CreateThread(firmware_thread, "app_main", NULL);
    if (!fw) {
        fprintf(stderr, "SDL_CreateThread: %s\n", SDL_GetError());
        return 1;
    }

    int pressed = 0;
    int imu_drag = 0;
    int down_x = 0, down_y = 0;
    int last_x = 0, last_y = 0;
    Uint32 last_drag_ms = SDL_GetTicks();
    Uint32 last_tick_ms = last_drag_ms;
    Uint32 last_title_ms = last_drag_ms;
    Uint32 last_tap_ms = 0;
    int running = 1;

    printf("keys: 1 PWR, 2 PLUS, 3 BOOT. Click taps the fake CST816. Drag past a few pixels tilts the fake IMU.\n");
    fflush(stdout);

    while (running) {
        const Uint32 frame_start = SDL_GetTicks();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                pressed = 1;
                imu_drag = 0;
                down_x = e.button.x;
                down_y = e.button.y;
                last_x = e.button.x;
                last_y = e.button.y;
                last_drag_ms = SDL_GetTicks();
            } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                if (pressed && !imu_drag) {
                    const Uint32 now = SDL_GetTicks();
                    int double_click = (last_tap_ms != 0 && (now - last_tap_ms) < 300) ? 1 : 0;
                    int lx = down_x / BOARD_SIM_SCALE;
                    int ly = down_y / BOARD_SIM_SCALE;
                    board_sim_touch_on_tap(lx, ly, double_click);
                    last_tap_ms = now;
                }
                pressed = 0;
                imu_drag = 0;
            } else if (e.type == SDL_MOUSEMOTION && pressed) {
                const Uint32 now = SDL_GetTicks();
                float dt = (now - last_drag_ms) / 1000.0f;
                if (dt < 0.001f) {
                    dt = 0.001f;
                }
                int dx = e.motion.x - last_x;
                int dy = e.motion.y - last_y;
                last_x = e.motion.x;
                last_y = e.motion.y;
                last_drag_ms = now;
                if (!imu_drag) {
                    int adx = e.motion.x - down_x;
                    int ady = e.motion.y - down_y;
                    if (adx < 0) {
                        adx = -adx;
                    }
                    if (ady < 0) {
                        ady = -ady;
                    }
                    if (adx + ady >= 8) {
                        imu_drag = 1;
                    }
                }
                if (imu_drag) {
                    board_sim_imu_on_drag(dx, dy, dt);
                }
            } else if ((e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) && !e.key.repeat) {
                int which = button_from_scancode(e.key.keysym.scancode);
                if (which >= 0) {
                    if (e.type == SDL_KEYDOWN) {
                        s_button_held[which] = 1;
                        s_button_until[which] = SDL_GetTicks() + BUTTON_HOLD_MS;
                    } else {
                        s_button_held[which] = 0;
                    }
                }
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                buttons_release_all();
            }
        }

        buttons_apply(SDL_GetTicks());

        const Uint32 now = SDL_GetTicks();
        if (now - last_title_ms >= 1000) {
            last_title_ms = now;
            format_title(title, sizeof(title));
            SDL_SetWindowTitle(win, title);
        }
        float tick_dt = (now - last_tick_ms) / 1000.0f;
        last_tick_ms = now;
        if (!imu_drag) {
            board_sim_imu_tick(tick_dt > 0.0f ? tick_dt : 0.033f);
        }

        board_sim_gram_copy(s_present);
        uint32_t *pixels = NULL;
        int pitch = 0;
        if (SDL_LockTexture(tex, NULL, (void **)&pixels, &pitch) == 0) {
            if (pitch == BOARD_SIM_LCD_W * 4) {
                rgb565_to_rgba8888(s_present, pixels, BOARD_SIM_LCD_W * BOARD_SIM_LCD_H);
            } else {
                uint32_t row[BOARD_SIM_LCD_W];
                for (int y = 0; y < BOARD_SIM_LCD_H; y++) {
                    rgb565_to_rgba8888(&s_present[y * BOARD_SIM_LCD_W], row, BOARD_SIM_LCD_W);
                    memcpy((uint8_t *)pixels + y * pitch, row, sizeof(row));
                }
            }
            SDL_UnlockTexture(tex);
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        const Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < 33) {
            SDL_Delay(33 - elapsed);
        }
    }

    SDL_DetachThread(fw);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
