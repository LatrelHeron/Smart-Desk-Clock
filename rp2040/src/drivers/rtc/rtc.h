#pragma once

#include <cstdint>
#include <cstdint>

#include "hardware/i2c.h"

struct DateTime
{
    uint16_t year = 2000;
    uint8_t month = 1;
    uint8_t day = 1;
    uint8_t weekday = 1;

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    bool valid = false;
};

class INS5699S
{
public:
    explicit INS5699S(i2c_inst_t *i2c_port);

    bool initialise();

    bool is_connected() const;

    DateTime read_datetime();

    bool set_datetime(const DateTime &date_time);

    bool read_raw_time_registers(uint8_t registers[7]);

private:
    static constexpr uint8_t I2C_ADDRESS = 0x32;
    static constexpr uint8_t REG_SECONDS = 0x00;

    i2c_inst_t *i2c_port_;
    bool connected_;

    bool read_registers(
        uint8_t start_register,
        uint8_t *buffer,
        std::size_t length);

    bool write_registers(
        uint8_t start_register,
        const uint8_t *data,
        std::size_t length);

    static uint8_t bcd_to_decimal(uint8_t value);
    static uint8_t decimal_to_bcd(uint8_t value);

    static bool is_valid_bcd(uint8_t value);

    static bool validate_datetime(const DateTime &date_time);
};