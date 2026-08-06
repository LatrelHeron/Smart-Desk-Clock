#pragma once

#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pico/types.h"

namespace board
{
    // I2C bus for the RTC and temp/humidity sensor.
    inline i2c_inst_t *const I2C_PORT = i2c0;

    constexpr uint I2C_SDA_PIN = 16;
    constexpr uint I2C_SCL_PIN = 17;

    constexpr uint I2C_BAUD_RATE = 100000;

    // SPI bus for the e-paper display.
    inline spi_inst_t *const EPD_SPI = spi0;

    constexpr uint EPD_MOSI_PIN = 19;
    constexpr uint EPD_SCK_PIN = 22;
    constexpr uint EPD_CS_PIN = 21;
    constexpr uint EPD_DC_PIN = 23;
    constexpr uint EPD_RST_PIN = 24;
    constexpr uint EPD_BUSY_PIN = 25;

    // Bus for SD card.
    constexpr uint SD_DAT3_PIN = 5;
    constexpr uint SD_CLK_PIN = 6;
    constexpr uint SD_CMD_PIN = 7;
    constexpr uint SD_DAT0_PIN = 8;
    constexpr uint SD_CD_PIN = 9;

    constexpr uint SD_BAUD_RATE = 1000000;

}

#define LED_PIN 14
#define LED_COUNT 12
#define DATA_FILE "data.txt"
#define CALENDAR_FILE "calendar.txt"