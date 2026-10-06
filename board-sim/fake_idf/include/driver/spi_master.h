#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPI1_HOST = 0,
    SPI2_HOST = 1,
    SPI3_HOST = 2,
} spi_host_device_t;

typedef int spi_dma_chan_t;

#define SPI_DMA_DISABLED 0
#define SPI_DMA_CH_AUTO 3

/* Field names match the IDF spi_bus_config_t members raster sets. */
typedef struct {
    int mosi_io_num;
    int miso_io_num;
    int sclk_io_num;
    int quadwp_io_num;
    int quadhd_io_num;
    int data4_io_num;
    int data5_io_num;
    int data6_io_num;
    int data7_io_num;
    int max_transfer_sz;
} spi_bus_config_t;

static inline esp_err_t spi_bus_initialize(spi_host_device_t host,
                                           const spi_bus_config_t *bus_config,
                                           spi_dma_chan_t dma_chan)
{
    (void)host;
    (void)bus_config;
    (void)dma_chan;
    return ESP_OK;
}

#ifdef __cplusplus
}
#endif
