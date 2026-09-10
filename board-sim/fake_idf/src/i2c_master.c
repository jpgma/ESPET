#include "driver/i2c_master.h"

#include "board_sim.h"

#include <stdlib.h>

#define QMI8658_ADDR_HI 0x6B
#define QMI8658_ADDR_LO 0x6A

struct i2c_master_bus_t {
    int dummy;
};

struct i2c_master_dev_t {
    uint16_t addr;
};

esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *bus_config,
                             i2c_master_bus_handle_t *ret_bus)
{
    (void)bus_config;
    if (!ret_bus) {
        return ESP_ERR_INVALID_ARG;
    }
    *ret_bus = calloc(1, sizeof(struct i2c_master_bus_t));
    return *ret_bus ? ESP_OK : ESP_FAIL;
}

esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,
                                    const i2c_device_config_t *dev_config,
                                    i2c_master_dev_handle_t *ret_handle)
{
    (void)bus;
    if (!dev_config || !ret_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    struct i2c_master_dev_t *dev = calloc(1, sizeof(*dev));
    if (!dev) {
        return ESP_FAIL;
    }
    dev->addr = dev_config->device_address;
    *ret_handle = dev;
    return ESP_OK;
}

static int is_qmi(uint16_t addr)
{
    return addr == QMI8658_ADDR_HI || addr == QMI8658_ADDR_LO;
}

esp_err_t i2c_master_transmit(i2c_master_dev_handle_t handle, const uint8_t *write_buffer,
                              size_t write_size, int xfer_timeout_ms)
{
    (void)xfer_timeout_ms;
    if (!handle || !write_buffer) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!is_qmi(handle->addr)) {
        return ESP_ERR_NOT_FOUND;
    }
    return board_sim_imu_i2c_tx(write_buffer, write_size) == 0 ? ESP_OK : ESP_FAIL;
}

esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t handle, const uint8_t *write_buffer,
                                      size_t write_size, uint8_t *read_buffer, size_t read_size,
                                      int xfer_timeout_ms)
{
    (void)xfer_timeout_ms;
    if (!handle || !read_buffer) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!is_qmi(handle->addr)) {
        return ESP_ERR_NOT_FOUND;
    }
    return board_sim_imu_i2c_txrx(write_buffer, write_size, read_buffer, read_size) == 0 ? ESP_OK
                                                                                         : ESP_FAIL;
}
