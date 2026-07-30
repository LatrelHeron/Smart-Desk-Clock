#include "temp_sensor.h"

#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <thread>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

namespace
{
    // Possible SEN0546 / CHT8305 addresses.
    constexpr std::array<uint8_t, 4> POSSIBLE_ADDRESSES = {
        0x40,
        0x44,
        0x48,
        0x4C};

    // CHT8305 measurement register.
    constexpr uint8_t MEASUREMENT_REGISTER = 0x00;

    // Manufacturer ID register used to test whether the device responds.
    constexpr uint8_t MANUFACTURER_ID_REGISTER = 0xFE;

    constexpr auto CONVERSION_DELAY =
        std::chrono::milliseconds(20);
}

Sen0546::Sen0546(const std::string &i2c_device)
    : i2c_device_(i2c_device),
      i2c_file_(-1),
      sensor_address_(0),
      initialised_(false)
{
}

Sen0546::~Sen0546()
{
    if (i2c_file_ >= 0)
    {
        close(i2c_file_);
        i2c_file_ = -1;
    }
}

void Sen0546::set_error(const std::string &message)
{
    last_error_ = message;
}

bool Sen0546::select_address(uint8_t address)
{
    if (i2c_file_ < 0)
    {
        set_error("I2C device is not open.");
        return false;
    }

    if (ioctl(i2c_file_, I2C_SLAVE, address) < 0)
    {
        std::ostringstream message;

        message
            << "Could not select I2C address 0x"
            << std::hex
            << static_cast<int>(address)
            << ": "
            << std::strerror(errno);

        set_error(message.str());
        return false;
    }

    return true;
}

bool Sen0546::device_responds(uint8_t address)
{
    if (!select_address(address))
    {
        return false;
    }

    uint8_t register_address = MANUFACTURER_ID_REGISTER;

    ssize_t written = write(
        i2c_file_,
        &register_address,
        sizeof(register_address));

    return written == static_cast<ssize_t>(
                          sizeof(register_address));
}

bool Sen0546::initialise()
{
    initialised_ = false;
    sensor_address_ = 0;
    last_error_.clear();

    if (i2c_file_ >= 0)
    {
        close(i2c_file_);
        i2c_file_ = -1;
    }

    i2c_file_ = open(
        i2c_device_.c_str(),
        O_RDWR);

    if (i2c_file_ < 0)
    {
        set_error(
            "Could not open " +
            i2c_device_ +
            ": " +
            std::strerror(errno));

        return false;
    }

    for (uint8_t address : POSSIBLE_ADDRESSES)
    {
        if (device_responds(address))
        {
            sensor_address_ = address;
            initialised_ = true;
            last_error_.clear();

            return true;
        }
    }

    set_error(
        "SEN0546 was not detected at addresses "
        "0x40, 0x44, 0x48 or 0x4C.");

    close(i2c_file_);
    i2c_file_ = -1;

    return false;
}

bool Sen0546::read(Sen0546Reading &reading)
{
    if (!initialised_)
    {
        set_error(
            "SEN0546 has not been initialised.");

        return false;
    }

    if (!select_address(sensor_address_))
    {
        return false;
    }

    uint8_t register_address = MEASUREMENT_REGISTER;

    ssize_t written = write(
        i2c_file_,
        &register_address,
        sizeof(register_address));

    if (written != static_cast<ssize_t>(
                       sizeof(register_address)))
    {
        set_error(
            "Failed to start SEN0546 measurement: " +
            std::string(std::strerror(errno)));

        return false;
    }

    std::this_thread::sleep_for(CONVERSION_DELAY);

    uint8_t data[4] = {};

    ssize_t received = ::read(
        i2c_file_,
        data,
        sizeof(data));

    if (received != static_cast<ssize_t>(
                        sizeof(data)))
    {
        set_error(
            "Failed to read four bytes from SEN0546: " +
            std::string(std::strerror(errno)));

        return false;
    }

    uint16_t raw_temperature =
        (static_cast<uint16_t>(data[0]) << 8) |
        static_cast<uint16_t>(data[1]);

    uint16_t raw_humidity =
        (static_cast<uint16_t>(data[2]) << 8) |
        static_cast<uint16_t>(data[3]);

    reading.temperature_c =
        (165.0f *
         static_cast<float>(raw_temperature) /
         65535.0f) -
        40.0f;

    reading.humidity_percent =
        100.0f *
        static_cast<float>(raw_humidity) /
        65535.0f;

    // Basic validation against physically possible values.
    if (
        reading.temperature_c < -40.0f ||
        reading.temperature_c > 125.0f)
    {
        set_error(
            "Temperature reading was outside the sensor range.");

        return false;
    }

    if (
        reading.humidity_percent < 0.0f ||
        reading.humidity_percent > 100.0f)
    {
        set_error(
            "Humidity reading was outside the sensor range.");

        return false;
    }

    last_error_.clear();

    return true;
}

bool Sen0546::is_initialised() const
{
    return initialised_;
}

uint8_t Sen0546::address() const
{
    return sensor_address_;
}

const std::string &Sen0546::last_error() const
{
    return last_error_;
}