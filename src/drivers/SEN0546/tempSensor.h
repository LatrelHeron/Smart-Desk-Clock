#ifndef SEN0546_H
#define SEN0546_H

#include <cstdint>
#include <string>

struct Sen0546Reading
{
    float temperature_c;
    float humidity_percent;
};

class Sen0546
{
public:
    // Default Raspberry Pi I2C bus is /dev/i2c-1.
    explicit Sen0546(
        const std::string &i2c_device = "/dev/i2c-1");

    ~Sen0546();

    // Prevent accidental copying of the file descriptor.
    Sen0546(const Sen0546 &) = delete;
    Sen0546 &operator=(const Sen0546 &) = delete;

    // Searches the supported SEN0546 addresses and opens the sensor.
    bool initialise();

    // Reads temperature and relative humidity.
    bool read(Sen0546Reading &reading);

    bool is_initialised() const;

    uint8_t address() const;

    const std::string &last_error() const;

private:
    bool select_address(uint8_t address);
    bool device_responds(uint8_t address);
    void set_error(const std::string &message);

    std::string i2c_device_;
    int i2c_file_;
    uint8_t sensor_address_;
    bool initialised_;
    std::string last_error_;
};

#endif