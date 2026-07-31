#include "temp_sensor.h"

#include <cstddef>
#include <cstdio>

#include "pico/stdlib.h"

SEN0546::SEN0546(i2c_inst_t *i2c_port)
    : i2c_port_(i2c_port),
      sensor_type_(SensorType::Unknown),
      address_(0),
      connected_(false)
{
}

bool SEN0546::initialise()
{
    connected_ = false;
    sensor_type_ = SensorType::Unknown;
    address_ = 0;

    // Try the older CHT8305 version first, it normally responds at address 0x40.
    if (probe_address(CHT8305_ADDRESS))
    {
        sensor_type_ = SensorType::CHT8305;
        address_ = CHT8305_ADDRESS;
        connected_ = true;

        printf(
            "SEN0546 detected: CHT8305 at address 0x%02X\n",
            address_);

        return true;
    }

    // Try the newer CHT832X version, it normally responds at address 0x44.
    if (probe_address(CHT832X_ADDRESS))
    {
        sensor_type_ = SensorType::CHT832X;
        address_ = CHT832X_ADDRESS;
        connected_ = true;

        printf(
            "SEN0546 detected: CHT832X at address 0x%02X\n",
            address_);

        return true;
    }

    printf(
        "ERROR: SEN0546 not detected at 0x40 or 0x44\n");

    return false;
}

bool SEN0546::probe_address(uint8_t address)
{
    // An empty write checks whether a device acknowledges the supplied I2C address
    const int result = i2c_write_blocking(
        i2c_port_,
        address,
        nullptr,
        0,
        false);

    return result >= 0;
}

EnvironmentData SEN0546::read()
{
    if (!connected_)
    {
        printf(
            "ERROR: SEN0546 read attempted before initialisation\n");

        return {};
    }

    switch (sensor_type_)
    {
    case SensorType::CHT8305:
        return read_cht8305();

    case SensorType::CHT832X:
        return read_cht832x();

    case SensorType::Unknown:
    default:
        return {};
    }
}

EnvironmentData SEN0546::read_cht8305()
{
    EnvironmentData result{};

    /*
    CHT8305 register 0x00 contains:
    - two bytes of temperature
    - two bytes of humidity
    */
    const uint8_t register_address = 0x00;

    const int write_result = i2c_write_blocking(
        i2c_port_,
        address_,
        &register_address,
        1,
        true);

    if (write_result != 1)
    {
        printf(
            "ERROR: CHT8305 register selection failed: %d\n",
            write_result);

        return result;
    }

    // DFRobot's reference example allows time before reading.
    sleep_ms(20);

    uint8_t buffer[4] = {0};

    const int read_result = i2c_read_blocking(
        i2c_port_,
        address_,
        buffer,
        sizeof(buffer),
        false);

    if (read_result != static_cast<int>(sizeof(buffer)))
    {
        printf(
            "ERROR: CHT8305 read failed: expected 4 bytes, got %d\n",
            read_result);

        return result;
    }

    const uint16_t raw_temperature =
        static_cast<uint16_t>(
            (static_cast<uint16_t>(buffer[0]) << 8) |
            buffer[1]);

    const uint16_t raw_humidity =
        static_cast<uint16_t>(
            (static_cast<uint16_t>(buffer[2]) << 8) |
            buffer[3]);

    // CHT8305 conversion formulas supplied by DFRobot.
    result.temperature_c =
        (static_cast<float>(raw_temperature) * 165.0f /
         65535.0f) -
        40.0f;

    result.humidity_percent =
        static_cast<float>(raw_humidity) * 100.0f /
        65535.0f;

    result.valid = readings_are_reasonable(
        result.temperature_c,
        result.humidity_percent);

    if (!result.valid)
    {
        printf(
            "ERROR: CHT8305 returned unreasonable values\n");
    }

    return result;
}

EnvironmentData SEN0546::read_cht832x()
{
    EnvironmentData result{};

    // CHT832X single-shot measurement command.
    const uint8_t command[2] = {
        0x24,
        0x00};

    const int write_result = i2c_write_blocking(
        i2c_port_,
        address_,
        command,
        sizeof(command),
        false);

    if (write_result != static_cast<int>(sizeof(command)))
    {
        printf(
            "ERROR: CHT832X measurement command failed: %d\n",
            write_result);

        return result;
    }

    // Give the sensor enough time to complete the conversion.
    sleep_ms(60);

    uint8_t buffer[6] = {0};

    const int read_result = i2c_read_blocking(
        i2c_port_,
        address_,
        buffer,
        sizeof(buffer),
        false);

    if (read_result != static_cast<int>(sizeof(buffer)))
    {
        printf(
            "ERROR: CHT832X read failed: expected 6 bytes, got %d\n",
            read_result);

        return result;
    }

    /*
    CHT832X data format:
    buffer[0] temperature MSB
    buffer[1] temperature LSB
    buffer[2] temperature CRC
    buffer[3] humidity MSB
    buffer[4] humidity LSB
    buffer[5] humidity CRC
    */

    const uint8_t expected_temperature_crc =
        calculate_crc8(buffer, 2);

    const uint8_t expected_humidity_crc =
        calculate_crc8(&buffer[3], 2);

    if (buffer[2] != expected_temperature_crc)
    {
        printf(
            "ERROR: CHT832X temperature CRC mismatch\n");

        return result;
    }

    if (buffer[5] != expected_humidity_crc)
    {
        printf(
            "ERROR: CHT832X humidity CRC mismatch\n");

        return result;
    }

    const uint16_t raw_temperature =
        static_cast<uint16_t>(
            (static_cast<uint16_t>(buffer[0]) << 8) |
            buffer[1]);

    const uint16_t raw_humidity =
        static_cast<uint16_t>(
            (static_cast<uint16_t>(buffer[3]) << 8) |
            buffer[4]);

    // CHT832X conversion formulas supplied by DFRobot.
    result.temperature_c =
        -45.0f +
        175.0f *
            (static_cast<float>(raw_temperature) / 65535.0f);

    result.humidity_percent =
        100.0f *
        (static_cast<float>(raw_humidity) / 65535.0f);

    result.valid = readings_are_reasonable(
        result.temperature_c,
        result.humidity_percent);

    if (!result.valid)
    {
        printf(
            "ERROR: CHT832X returned unreasonable values\n");
    }

    return result;
}

bool SEN0546::is_connected() const
{
    return connected_;
}

const char *SEN0546::sensor_name() const
{
    switch (sensor_type_)
    {
    case SensorType::CHT8305:
        return "CHT8305";

    case SensorType::CHT832X:
        return "CHT832X";

    case SensorType::Unknown:
    default:
        return "Unknown";
    }
}

bool SEN0546::readings_are_reasonable(
    float temperature_c,
    float humidity_percent)
{
    /*
    SEN0546's documented operating ranges are:
    - Temperature: -40 C to 125 C
    - Humidity: 0% to 100% RH
    */
    const bool valid_temperature =
        temperature_c >= -40.0f &&
        temperature_c <= 125.0f;

    const bool valid_humidity =
        humidity_percent >= 0.0f &&
        humidity_percent <= 100.0f;

    return valid_temperature && valid_humidity;
}

uint8_t SEN0546::calculate_crc8(
    const uint8_t *data,
    std::size_t length)
{
    /*
    Common CRC-8 calculation used by this style of temperature/humidity sensor.
    Polynomial: 0x31
    Initial value: 0xFF
    */
    uint8_t crc = 0xFF;

    for (std::size_t byte_index = 0;
         byte_index < length;
         ++byte_index)
    {
        crc ^= data[byte_index];

        for (int bit = 0; bit < 8; ++bit)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc =
                    static_cast<uint8_t>(
                        (crc << 1U) ^ 0x31U);
            }
            else
            {
                crc =
                    static_cast<uint8_t>(crc << 1U);
            }
        }
    }

    return crc;
}