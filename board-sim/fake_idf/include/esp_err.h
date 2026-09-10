#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t esp_err_t;

#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_INVALID_STATE 0x103
#define ESP_ERR_NOT_FOUND 0x105

#define ESP_ERROR_CHECK(x)                                                         \
    do {                                                                           \
        esp_err_t _err_ = (x);                                                     \
        if (_err_ != ESP_OK) {                                                     \
            printf("ESP_ERROR_CHECK failed: 0x%x at %s:%d\n", (int)_err_, __FILE__, \
                   __LINE__);                                                      \
            abort();                                                               \
        }                                                                          \
    } while (0)
