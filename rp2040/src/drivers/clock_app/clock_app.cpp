#include "app/clock_app.h"

#include <cstdio>
#include <cstdint>

namespace app
{
    bool draw_clock_screen(
        epaper::Epaper3in7 &display,
        const DateTime &time,
        const EnvironmentData &environment
    )
    {
        static uint8_t image[epaper::BUFFER_SIZE];

        epaper::buffer_clear(image, false);

        epaper::draw_rect(
            image,
            5,
            5,
            epaper::WIDTH - 10,
            epaper::HEIGHT - 10,
            true,
            false
        );

        epaper::draw_text(
            image,
            20,
            20,
            "SMART DESK CLOCK",
            3
        );

        char time_text[32];
        char date_text[32];
        char temperature_text[32];
        char humidity_text[32];

        if (time.valid)
        {
            std::snprintf(
                time_text,
                sizeof(time_text),
                "%02u:%02u:%02u",
                time.hour,
                time.minute,
                time.second
            );

            std::snprintf(
                date_text,
                sizeof(date_text),
                "%02u/%02u/%04u",
                time.day,
                time.month,
                time.year
            );
        }
        else
        {
            std::snprintf(
                time_text,
                sizeof(time_text),
                "TIME INVALID"
            );

            std::snprintf(
                date_text,
                sizeof(date_text),
                "--/--/----"
            );
        }

        if (environment.valid)
        {
            std::snprintf(
                temperature_text,
                sizeof(temperature_text),
                "TEMP %.1F C",
                environment.temperature_c
            );

            std::snprintf(
                humidity_text,
                sizeof(humidity_text),
                "HUM %.1F PERCENT",
                environment.humidity_percent
            );
        }
        else
        {
            std::snprintf(
                temperature_text,
                sizeof(temperature_text),
                "TEMP --.- C"
            );

            std::snprintf(
                humidity_text,
                sizeof(humidity_text),
                "HUM --.- PERCENT"
            );
        }

        epaper::draw_text(
            image,
            30,
            90,
            time_text,
            5
        );

        epaper::draw_text(
            image,
            30,
            150,
            date_text,
            3
        );

        epaper::draw_text(
            image,
            30,
            230,
            temperature_text,
            3
        );

        epaper::draw_text(
            image,
            30,
            290,
            humidity_text,
            3
        );

        return display.display(image);
    }
}