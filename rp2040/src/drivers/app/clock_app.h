#pragma once

#include <cstdint>
#include <string>

#include "drivers/epaper/epaper_3in7.h"
#include "drivers/rtc/rtc.h"
#include "drivers/SEN0546/temp_sensor.h"

namespace app
{
    struct EventData
    {
        std::string name;
        std::string date;
        std::string time;

        bool valid = false;
    };
    void build_vertical_screen(
        std::uint8_t *image,
        const DateTime &time,
        const EnvironmentData &environment,
        const EventData &event);

    void build_horizontal_screen(
        std::uint8_t *image,
        const DateTime &time,
        const EnvironmentData &environment,
        const EventData &event);
}