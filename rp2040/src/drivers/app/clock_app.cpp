#include "drivers/app/clock_app.h"

#include <cstdint>
#include <cstdio>

namespace app
{
    bool draw_clock_screen(
        epaper::Epaper3in7 &display,
        const DateTime &time,
        const EnvironmentData &environment)
    {
        static uint8_t image[epaper::BUFFER_SIZE];

        epaper::buffer_clear(image, false);

        // Outer border.
        epaper::draw_rect(
            image,
            5,
            5,
            epaper::WIDTH - 10,
            epaper::HEIGHT - 10,
            true,
            false);

        // 16 characters × 6 pixels × scale 2 = 192 pixels.
        epaper::draw_text(
            image,
            24,
            20,
            "SMART DESK CLOCK",
            2);

        char time_text[32];
        char date_text[32];
        char temperature_text[32];
        char humidity_text[32];

        if (time.valid)
        {
            // Use hyphens because the current font does not contain ':'.
            std::snprintf(
                time_text,
                sizeof(time_text),
                "%02u-%02u-%02u",
                static_cast<unsigned>(time.hour),
                static_cast<unsigned>(time.minute),
                static_cast<unsigned>(time.second));

            // Current font also does not contain '/'.
            std::snprintf(
                date_text,
                sizeof(date_text),
                "%02u-%02u-%04u",
                static_cast<unsigned>(time.day),
                static_cast<unsigned>(time.month),
                static_cast<unsigned>(time.year));
        }
        else
        {
            std::snprintf(
                time_text,
                sizeof(time_text),
                "TIME INVALID");

            std::snprintf(
                date_text,
                sizeof(date_text),
                "DATE INVALID");
        }

        if (environment.valid)
        {
            std::snprintf(
                temperature_text,
                sizeof(temperature_text),
                "TEMP %.1f C",
                static_cast<double>(environment.temperature_c));

            std::snprintf(
                humidity_text,
                sizeof(humidity_text),
                "HUM %.1f PCT",
                static_cast<double>(environment.humidity_percent));
        }
        else
        {
            std::snprintf(
                temperature_text,
                sizeof(temperature_text),
                "TEMP INVALID");

            std::snprintf(
                humidity_text,
                sizeof(humidity_text),
                "HUM INVALID");
        }

        epaper::draw_text(
            image,
            20,
            90,
            time_text,
            4);

        epaper::draw_text(
            image,
            35,
            155,
            date_text,
            2);

        epaper::draw_text(
            image,
            20,
            235,
            temperature_text,
            3);

        epaper::draw_text(
            image,
            20,
            305,
            humidity_text,
            3);

        return display.display(image);
    }
}