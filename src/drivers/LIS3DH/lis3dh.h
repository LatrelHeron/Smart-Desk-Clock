#pragma once

#include <stdint.h>

struct lis3dh_raw_t {
    int16_t x;
    int16_t y;
    int16_t z;
};

struct lis3dh_g_t {
    float x;
    float y;
    float z;
};

bool lis3dh_init();
lis3dh_raw_t lis3dh_read_raw();
lis3dh_g_t lis3dh_read_g();