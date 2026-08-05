#pragma once

#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pico/types.h"

namespace board
{
    inline i2c_inst_t *const I2C_PORT = i2c0;

    constexpr uint I2C_SDA_PIN = 16;
    constexpr uint I2C_SCL_PIN = 17;

    constexpr uint I2C_BAUD_RATE = 100000;

    inline spi_inst_t *const EPD_SPI = spi0;

    constexpr uint EPD_MOSI_PIN  = 19;
    constexpr uint EPD_SCK_PIN   = 22;
    constexpr uint EPD_CS_PIN    = 21;
    constexpr uint EPD_DC_PIN    = 23;
    constexpr uint EPD_RST_PIN   = 24;
    constexpr uint EPD_BUSY_PIN  = 25;
}


#define LED_PIN 14
#define LED_COUNT 12