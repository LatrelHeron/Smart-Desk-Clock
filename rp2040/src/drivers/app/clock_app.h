#pragma once

#include "drivers/epaper/epaper_3in7.h"
#include "drivers/rtc/rtc.h"
#include "drivers/SEN0546/temp_sensor.h"

namespace app
{
    bool draw_clock_screen(
        epaper::Epaper3in7 &display,
        const DateTime &time,
        const EnvironmentData &environment);
}