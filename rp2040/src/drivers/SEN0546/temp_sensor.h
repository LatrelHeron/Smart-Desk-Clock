#pragma once

#include <cstdint>
#include <cstddef>

#include "hardware/i2c.h"

// Return values for application
struct EnvironmentData
{
    float temperature_c = 0.0f;
    float humidity_percent = 0.0f;
    bool valid = false;
};

class SEN0546
{
public:
    // Construct sensor driver
    explicit SEN0546(i2c_inst_t *i2c_port);

    // Detect and initialise sensor
    bool initialise();

    // Read temp and humidity
    EnvironmentData read();

    // Indicates if initialisation was a success
    bool is_connected() const;

    // Return detected sensor name
    const char *sensor_name() const;

private:
    enum class SensorType
    {
        Unknown,
        CHT8305,
        CHT832X
    };

    static constexpr uint8_t CHT8305_ADDRESS = 0x40;
    static constexpr uint8_t CHT832X_ADDRESS = 0x44;

    i2c_inst_t *i2c_port_;
    SensorType sensor_type_;
    uint8_t address_;
    bool connected_;

    bool probe_address(uint8_t address);
    EnvironmentData read_cht8305();
    EnvironmentData read_cht832x();

    static bool readings_are_reasonable(
        float temperature_c,
        float humidity_percent);

    static uint8_t calculate_crc8(
        const uint8_t *data,
        std::size_t length);
};