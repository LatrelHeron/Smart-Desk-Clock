#pragma once

#include <cstdint>

#include "drivers/epaper/epaper_3in7.h"
#include "drivers/rtc/rtc.h"
#include "drivers/SEN0546/temp_sensor.h"

namespace app
{
    void build_home_screen(
        std::uint8_t* image,
        const DateTime &time,
        const EnvironmentData &environment);
}