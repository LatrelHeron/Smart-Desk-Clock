#include <opencv2/opencv.hpp>
#include <sys/time.h>

#include <chrono>
#include <cstdio>
#include <thread>

#include ".h"

int main()
{
    Sen0546 temperature_sensor;

    if (!temperature_sensor.initialise()) {
        std::fprintf(
            stderr,
            "SEN0546 initialisation failed: %s\n",
            temperature_sensor.last_error().c_str()
        );

        return 1;
    }

    std::printf(
        "SEN0546 found at I2C address 0x%02X\n",
        temperature_sensor.address()
    );

    while (true) {
        Sen0546Reading reading{};

        if (temperature_sensor.read(reading)) {
            std::printf(
                "Temperature: %.2f C | "
                "Humidity: %.2f %%RH\n",
                reading.temperature_c,
                reading.humidity_percent
            );
        }
        else {
            std::fprintf(
                stderr,
                "SEN0546 reading failed: %s\n",
                temperature_sensor.last_error().c_str()
            );
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(2)
        );
    }

    return 0;
}