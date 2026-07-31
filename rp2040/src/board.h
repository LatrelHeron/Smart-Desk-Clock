#pragma once

#include "hardware/i2c.h"
#include "pico/types.h"

namespace board
{
    inline i2c_inst_t *const I2C_PORT = i2c0;

    constexpr uint I2C_SDA_PIN = 16;
    constexpr uint I2C_SCL_PIN = 17;

    constexpr uint I2C_BAUD_RATE = 100000;
}


#define LED_PIN 14
#define LED_COUNT 12