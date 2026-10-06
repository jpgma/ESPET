#include "platform.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "esp_netif.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"

static const char *TAG = "raster";

static SemaphoreHandle_t s_spi_done;
static int s_band_in_flight;
static int64_t s_last_frame_start_us;
static int s_has_started;

static bool spi_done(esp_lcd_panel_io_handle_t panel_io,
                     esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    (void)panel_io;
    (void)edata;
    (void)user_ctx;
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR(s_spi_done, &wake);
    return wake == pdTRUE;
}

static void init_wifi(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void platform_prepare(const void *indexed, size_t indexed_bytes,
                      const void *palette, size_t palette_bytes,
                      const void *bands, size_t bands_bytes)
{
    (void)indexed_bytes;
    (void)palette_bytes;
    (void)bands_bytes;

    init_wifi();

    /* sdkconfig: octal PSRAM 80 MHz, quad flash 80 MHz,
     * Bluetooth off, Wi-Fi task pinned to core 0
     * (CONFIG_ESP_WIFI_TASK_PINNED_TO_CORE_0). */
    ESP_LOGI(TAG, "free internal %u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "indexed %s palette %s bands %s",
             esp_ptr_internal(indexed) ? "internal" : "external",
             esp_ptr_internal(palette) ? "internal" : "external",
             esp_ptr_internal(bands) ? "internal" : "external");
}

void platform_spi_arm(esp_lcd_panel_io_handle_t io)
{
    s_spi_done = xSemaphoreCreateBinary();
    esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = spi_done,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io, &cbs, NULL));
}

void platform_wait_dma(void)
{
    if (s_band_in_flight) {
        xSemaphoreTake(s_spi_done, portMAX_DELAY);
        s_band_in_flight = 0;
    }
}

void platform_dma_queued(void)
{
    s_band_in_flight = 1;
}

void platform_pace(int64_t period_us)
{
    int64_t next = s_last_frame_start_us + period_us;
    int64_t now = esp_timer_get_time();
    if (s_has_started && now < next) {
        esp_rom_delay_us((uint32_t)(next - now));
        now = esp_timer_get_time();
    }
    s_last_frame_start_us = now;
    s_has_started = 1;
}
