#include "rtc.h"

#include <cstddef>
#include <cstdio>

INS5699S::INS5699S(i2c_inst_t *i2c_port)
    : i2c_port_(i2c_port),
      connected_(false)
{
}

bool INS5699S::initialise()
{
    connected_ = false;

    /*
     * Probe the device by writing the starting register address.
     *
     * This does not modify the RTC. It only checks whether a device
     * acknowledges address 0x32.
     */
    const uint8_t register_address = REG_SECONDS;

    const int result = i2c_write_blocking(
        i2c_port_,
        I2C_ADDRESS,
        &register_address,
        1,
        false);

    if (result != 1)
    {
        printf(
            "RTC not detected at I2C address 0x%02X, result=%d\n",
            I2C_ADDRESS,
            result);

        return false;
    }

    connected_ = true;

    printf(
        "RTC detected at I2C address 0x%02X\n",
        I2C_ADDRESS);

    return true;
}

bool INS5699S::is_connected() const
{
    return connected_;
}

DateTime INS5699S::read_datetime()
{
    DateTime result{};

    if (!connected_)
    {
        printf("RTC read attempted before initialise()\n");
        return result;
    }

    uint8_t raw[7] = {0};

    if (!read_raw_time_registers(raw))
    {
        printf("RTC time-register read failed\n");
        return result;
    }

    /*
     * Expected register order:
     *
     * 0x00 seconds
     * 0x01 minutes
     * 0x02 hours
     * 0x03 weekday
     * 0x04 day
     * 0x05 month
     * 0x06 year
     */

    const uint8_t raw_seconds = raw[0] & 0x7F;
    const uint8_t raw_minutes = raw[1] & 0x7F;
    const uint8_t raw_hours = raw[2] & 0x3F;
    const uint8_t raw_weekday = raw[3] & 0x7F;
    const uint8_t raw_day = raw[4] & 0x3F;
    const uint8_t raw_month = raw[5] & 0x1F;
    const uint8_t raw_year = raw[6];

    if (!is_valid_bcd(raw_seconds) ||
        !is_valid_bcd(raw_minutes) ||
        !is_valid_bcd(raw_hours) ||
        !is_valid_bcd(raw_day) ||
        !is_valid_bcd(raw_month) ||
        !is_valid_bcd(raw_year))
    {
        printf("RTC returned invalid BCD data\n");
        return result;
    }

    result.second = bcd_to_decimal(raw_seconds);
    result.minute = bcd_to_decimal(raw_minutes);
    result.hour = bcd_to_decimal(raw_hours);

    /*
     * Weekday is a one-hot bitfield in the related INS5699C:
     *
     * Sunday    0x01
     * Monday    0x02
     * Tuesday   0x04
     * Wednesday 0x08
     * Thursday  0x10
     * Friday    0x20
     * Saturday  0x40
     *
     * Store the raw weekday bitfield for now.
     */
    result.weekday = raw_weekday;

    result.day = bcd_to_decimal(raw_day);
    result.month = bcd_to_decimal(raw_month);
    result.year =
        static_cast<uint16_t>(
            2000U + bcd_to_decimal(raw_year));

    result.valid = validate_datetime(result);

    if (!result.valid)
    {
        printf(
            "RTC returned out-of-range date/time values\n");
    }

    return result;
}

bool INS5699S::set_datetime(const DateTime &date_time)
{
    if (!connected_)
    {
        printf("RTC write attempted before initialise()\n");
        return false;
    }

    if (!validate_datetime(date_time))
    {
        printf("Refusing to write invalid date/time\n");
        return false;
    }

    if (date_time.year < 2000 ||
        date_time.year > 2099)
    {
        printf("RTC only supports years 2000-2099\n");
        return false;
    }

    /*
     * Weekday must be one of:
     * 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40
     */
    const bool valid_weekday =
        date_time.weekday == 0x01 ||
        date_time.weekday == 0x02 ||
        date_time.weekday == 0x04 ||
        date_time.weekday == 0x08 ||
        date_time.weekday == 0x10 ||
        date_time.weekday == 0x20 ||
        date_time.weekday == 0x40;

    if (!valid_weekday)
    {
        printf("Invalid RTC weekday bitfield\n");
        return false;
    }

    uint8_t data[7];

    data[0] = decimal_to_bcd(date_time.second);
    data[1] = decimal_to_bcd(date_time.minute);
    data[2] = decimal_to_bcd(date_time.hour);
    data[3] = date_time.weekday;
    data[4] = decimal_to_bcd(date_time.day);
    data[5] = decimal_to_bcd(date_time.month);
    data[6] = decimal_to_bcd(
        static_cast<uint8_t>(date_time.year - 2000));

    return write_registers(
        REG_SECONDS,
        data,
        sizeof(data));
}

bool INS5699S::read_raw_time_registers(
    uint8_t registers[7])
{
    return read_registers(
        REG_SECONDS,
        registers,
        7);
}

bool INS5699S::read_registers(
    uint8_t start_register,
    uint8_t *buffer,
    std::size_t length)
{
    if (buffer == nullptr || length == 0)
    {
        return false;
    }

    /*
     * Select the starting register and retain the bus,
     * causing a repeated-start before the read.
     */
    const int write_result = i2c_write_blocking(
        i2c_port_,
        I2C_ADDRESS,
        &start_register,
        1,
        true);

    if (write_result != 1)
    {
        printf(
            "RTC register selection failed: %d\n",
            write_result);

        return false;
    }

    const int read_result = i2c_read_blocking(
        i2c_port_,
        I2C_ADDRESS,
        buffer,
        length,
        false);

    if (read_result != static_cast<int>(length))
    {
        printf(
            "RTC read failed: expected %u bytes, received %d\n",
            static_cast<unsigned>(length),
            read_result);

        return false;
    }

    return true;
}

bool INS5699S::write_registers(
    uint8_t start_register,
    const uint8_t *data,
    std::size_t length)
{
    if (data == nullptr || length == 0)
    {
        return false;
    }

    /*
     * One byte for the starting register followed by the data.
     * Seven time bytes means an eight-byte transmission.
     */
    constexpr std::size_t MAX_WRITE_LENGTH = 16;

    if (length + 1 > MAX_WRITE_LENGTH)
    {
        printf("RTC write exceeds local buffer size\n");
        return false;
    }

    uint8_t buffer[MAX_WRITE_LENGTH];

    buffer[0] = start_register;

    for (std::size_t index = 0;
         index < length;
         ++index)
    {
        buffer[index + 1] = data[index];
    }

    const int write_result = i2c_write_blocking(
        i2c_port_,
        I2C_ADDRESS,
        buffer,
        length + 1,
        false);

    if (write_result != static_cast<int>(length + 1))
    {
        printf(
            "RTC register write failed: expected %u bytes, wrote %d\n",
            static_cast<unsigned>(length + 1),
            write_result);

        return false;
    }

    return true;
}

uint8_t INS5699S::bcd_to_decimal(uint8_t value)
{
    return static_cast<uint8_t>(
        ((value >> 4U) * 10U) +
        (value & 0x0FU));
}

uint8_t INS5699S::decimal_to_bcd(uint8_t value)
{
    return static_cast<uint8_t>(
        ((value / 10U) << 4U) |
        (value % 10U));
}

bool INS5699S::is_valid_bcd(uint8_t value)
{
    const uint8_t lower_nibble = value & 0x0F;
    const uint8_t upper_nibble =
        static_cast<uint8_t>((value >> 4U) & 0x0F);

    return lower_nibble <= 9 &&
           upper_nibble <= 9;
}

bool INS5699S::validate_datetime(
    const DateTime &date_time)
{
    if (date_time.second > 59)
    {
        return false;
    }

    if (date_time.minute > 59)
    {
        return false;
    }

    if (date_time.hour > 23)
    {
        return false;
    }

    if (date_time.month < 1 ||
        date_time.month > 12)
    {
        return false;
    }

    if (date_time.day < 1 ||
        date_time.day > 31)
    {
        return false;
    }

    if (date_time.year < 2000 ||
        date_time.year > 2099)
    {
        return false;
    }

    return true;
}