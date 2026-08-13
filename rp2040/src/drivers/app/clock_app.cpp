#include "drivers/app/clock_app.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace app
{
    // ---------------------------------------------------------
    // Seven-segment clock drawing
    // ---------------------------------------------------------

    void draw_segment(
        uint8_t *image,
        int x,
        int y,
        int width,
        int height)
    {
        epaper::draw_rect(
            image,
            x,
            y,
            width,
            height,
            true,
            true);
    }

    void draw_digit(
        uint8_t *image,
        int x,
        int y,
        int digit,
        int width = 42,
        int height = 145,
        int thickness = 7)
    {
        // Segment layout:
        //
        //      A
        //    -----
        // F |     | B
        //   |  G  |
        //    -----
        // E |     | C
        //   |     |
        //    -----
        //      D

        static const bool segments[10][7] =
            {
                // A B C D E F G
                {1, 1, 1, 1, 1, 1, 0}, // 0
                {0, 1, 1, 0, 0, 0, 0}, // 1
                {1, 1, 0, 1, 1, 0, 1}, // 2
                {1, 1, 1, 1, 0, 0, 1}, // 3
                {0, 1, 1, 0, 0, 1, 1}, // 4
                {1, 0, 1, 1, 0, 1, 1}, // 5
                {1, 0, 1, 1, 1, 1, 1}, // 6
                {1, 1, 1, 0, 0, 0, 0}, // 7
                {1, 1, 1, 1, 1, 1, 1}, // 8
                {1, 1, 1, 1, 0, 1, 1}  // 9
            };

        if (digit < 0 || digit > 9)
        {
            return;
        }

        const int half = height / 2;

        // A
        if (segments[digit][0])
        {
            draw_segment(
                image,
                x + thickness,
                y,
                width - 2 * thickness,
                thickness);
        }

        // B
        if (segments[digit][1])
        {
            draw_segment(
                image,
                x + width - thickness,
                y + thickness,
                thickness,
                half - thickness);
        }

        // C
        if (segments[digit][2])
        {
            draw_segment(
                image,
                x + width - thickness,
                y + half,
                thickness,
                half - thickness);
        }

        // D
        if (segments[digit][3])
        {
            draw_segment(
                image,
                x + thickness,
                y + height - thickness,
                width - 2 * thickness,
                thickness);
        }

        // E
        if (segments[digit][4])
        {
            draw_segment(
                image,
                x,
                y + half,
                thickness,
                half - thickness);
        }

        // F
        if (segments[digit][5])
        {
            draw_segment(
                image,
                x,
                y + thickness,
                thickness,
                half - thickness);
        }

        // G
        if (segments[digit][6])
        {
            draw_segment(
                image,
                x + thickness,
                y + half - thickness / 2,
                width - 2 * thickness,
                thickness);
        }
    }

    void draw_period(
        uint8_t *image,
        uint8_t hour,
        int x,
        int y)
    {
        epaper::draw_text(
            image,
            x,
            y,
            hour >= 12 ? "PM" : "AM",
            2);
    }

    void draw_colon(
        uint8_t *image,
        int x,
        int y)
    {
        epaper::draw_rect(
            image,
            x,
            y + 47,
            8,
            8,
            true,
            true);

        epaper::draw_rect(
            image,
            x,
            y + 92,
            8,
            8,
            true,
            true);
    }

    void draw_large_time(
        uint8_t *image,
        uint8_t hour,
        uint8_t minute)
    {
        constexpr int DIGIT_WIDTH = 42;
        constexpr int DIGIT_HEIGHT = 145;
        constexpr int GAP = 6;
        constexpr int COLON_WIDTH = 8;

        uint8_t display_hour = hour % 12;

        if (display_hour == 0)
        {
            display_hour = 12;
        }

        const int h1 = display_hour / 10;
        const int h2 = display_hour % 10;
        const int m1 = minute / 10;
        const int m2 = minute % 10;

        constexpr int y = 70;

        if (display_hour < 10)
        {
            // 5:36 style — centred with no leading zero.
            constexpr int total_width =
                DIGIT_WIDTH * 3 +
                GAP * 3 +
                COLON_WIDTH;

            int x = (epaper::WIDTH - total_width) / 2;

            draw_digit(image, x, y, h2);
            x += DIGIT_WIDTH + GAP;

            draw_colon(image, x, y);
            x += COLON_WIDTH + GAP;

            draw_digit(image, x, y, m1);
            x += DIGIT_WIDTH + GAP;

            draw_digit(image, x, y, m2);
        }
        else
        {
            // 17:43 style.
            constexpr int total_width =
                DIGIT_WIDTH * 4 +
                GAP * 4 +
                COLON_WIDTH;

            int x = (epaper::WIDTH - total_width) / 2;

            draw_digit(image, x, y, h1);
            x += DIGIT_WIDTH + GAP;

            draw_digit(image, x, y, h2);
            x += DIGIT_WIDTH + GAP;

            draw_colon(image, x, y);
            x += COLON_WIDTH + GAP;

            draw_digit(image, x, y, m1);
            x += DIGIT_WIDTH + GAP;

            draw_digit(image, x, y, m2);
        }
    }

    void draw_large_time_horizontal(
        uint8_t *image,
        uint8_t hour,
        uint8_t minute)
    {
        uint8_t display_hour = hour % 12;

        if (display_hour == 0)
        {
            display_hour = 12;
        }

        const int h1 = display_hour / 10;
        const int h2 = display_hour % 10;
        const int m1 = minute / 10;
        const int m2 = minute % 10;

        constexpr int REGION_X = 0;
        constexpr int REGION_WIDTH = 250;

        constexpr int DIGIT_HEIGHT = 105;
        constexpr int THICKNESS = 7;

        constexpr int COLON_WIDTH = 8;

        constexpr int y = 48;

        // -----------------------------------------------------
        // Single digit hour e.g. 9:16
        // -----------------------------------------------------

        if (display_hour < 10)
        {
            constexpr int DIGIT_WIDTH = 58;
            constexpr int GAP = 7;

            constexpr int total_width =
                DIGIT_WIDTH * 3 +
                GAP * 3 +
                COLON_WIDTH;

            int x = REGION_X + (REGION_WIDTH - total_width) / 2;

            draw_digit(
                image,
                x,
                y,
                h2,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);

            x += DIGIT_WIDTH + GAP;

            draw_colon(
                image,
                x,
                y - 18);

            x += COLON_WIDTH + GAP;

            draw_digit(
                image,
                x,
                y,
                m1,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);

            x += DIGIT_WIDTH + GAP;

            draw_digit(
                image,
                x,
                y,
                m2,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);
        }
        else
        {
            constexpr int DIGIT_WIDTH = 48;
            constexpr int GAP = 6;

            constexpr int total_width =
                DIGIT_WIDTH * 4 +
                GAP * 4 +
                COLON_WIDTH;

            int x = REGION_X + (REGION_WIDTH - total_width) / 2;

            draw_digit(
                image,
                x,
                y,
                h1,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);

            x += DIGIT_WIDTH + GAP;

            draw_digit(
                image,
                x,
                y,
                h2,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);

            x += DIGIT_WIDTH + GAP;

            draw_colon(
                image,
                x,
                y - 15);

            x += COLON_WIDTH + GAP;

            draw_digit(
                image,
                x,
                y,
                m1,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);

            x += DIGIT_WIDTH + GAP;

            draw_digit(
                image,
                x,
                y,
                m2,
                DIGIT_WIDTH,
                DIGIT_HEIGHT,
                THICKNESS);
        }
    }

    void draw_temperature_value(
        uint8_t *image,
        int x,
        int y,
        float temperature)
    {
        // Example:
        //
        // 27 . 5  °C
        // ^^   ^
        // large small

        int whole =
            static_cast<int>(temperature);

        int decimal =
            static_cast<int>(
                temperature * 10.0f) %
            10;

        if (decimal < 0)
        {
            decimal = -decimal;
        }

        const int tens =
            (whole / 10) % 10;

        const int ones =
            whole % 10;

        constexpr int LARGE_WIDTH = 24;
        constexpr int LARGE_HEIGHT = 40;
        constexpr int LARGE_THICKNESS = 4;

        constexpr int SMALL_WIDTH = 13;
        constexpr int SMALL_HEIGHT = 24;
        constexpr int SMALL_THICKNESS = 3;

        // Whole-number digits.
        draw_digit(
            image,
            x,
            y,
            tens,
            LARGE_WIDTH,
            LARGE_HEIGHT,
            LARGE_THICKNESS);

        x += LARGE_WIDTH + 5;

        draw_digit(
            image,
            x,
            y,
            ones,
            LARGE_WIDTH,
            LARGE_HEIGHT,
            LARGE_THICKNESS);

        x += LARGE_WIDTH + 5;

        // Small decimal point.
        epaper::draw_rect(
            image,
            x,
            y + LARGE_HEIGHT - 5,
            4,
            4,
            true,
            true);

        x += 10;

        // Smaller decimal digit.
        draw_digit(
            image,
            x,
            y + 14,
            decimal,
            SMALL_WIDTH,
            SMALL_HEIGHT,
            SMALL_THICKNESS);

        x += SMALL_WIDTH + 6;

        // Degree symbol.
        epaper::draw_rect(
            image,
            x,
            y + 4,
            7,
            7,
            true,
            false);

        // C unit.
        epaper::draw_text(
            image,
            x + 11,
            y + 5,
            "C",
            2);
    }

    void draw_percent_symbol(
        uint8_t *image,
        int x,
        int y)
    {
        // Top square
        epaper::draw_rect(
            image,
            x,
            y,
            6,
            6,
            true,
            false);

        // Bottom square
        epaper::draw_rect(
            image,
            x + 14,
            y + 18,
            6,
            6,
            true,
            false);

        // Symmetrical diagonal slash
        for (int i = 0; i < 18; ++i)
        {
            const int px =
                x + 16 - i;

            const int py =
                y + 3 + i;

            // 2-pixel thickness
            epaper::set_pixel(
                image,
                px,
                py,
                true);

            epaper::set_pixel(
                image,
                px + 1,
                py,
                true);
        }
    }

    void draw_humidity_value(
        uint8_t *image,
        int x,
        int y,
        float humidity)
    {
        const int value =
            static_cast<int>(humidity + 0.5f);

        const int tens =
            (value / 10) % 10;

        const int ones =
            value % 10;

        constexpr int DIGIT_WIDTH = 24;
        constexpr int DIGIT_HEIGHT = 40;
        constexpr int THICKNESS = 4;

        draw_digit(
            image,
            x,
            y,
            tens,
            DIGIT_WIDTH,
            DIGIT_HEIGHT,
            THICKNESS);

        x += DIGIT_WIDTH + 5;

        draw_digit(
            image,
            x,
            y,
            ones,
            DIGIT_WIDTH,
            DIGIT_HEIGHT,
            THICKNESS);

        x += DIGIT_WIDTH + 10;

        // Use the custom symbol because '%' is not
        // currently part of the 5x7 text font.
        draw_percent_symbol(
            image,
            x,
            y + 9);
    }

    static void draw_hline(
        uint8_t *image,
        int x,
        int y,
        int width,
        bool black)
    {
        epaper::draw_rect(
            image,
            x,
            y,
            width,
            1,
            black,
            true);
    }

    static void draw_line(
        uint8_t *image,
        int x0,
        int y0,
        int x1,
        int y1,
        bool black)
    {
        int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
        int sx = (x0 < x1) ? 1 : -1;

        int dy = (y1 > y0) ? -(y1 - y0) : -(y0 - y1);
        int sy = (y0 < y1) ? 1 : -1;

        int error = dx + dy;

        while (true)
        {
            epaper::set_pixel(image, x0, y0, black);

            if (x0 == x1 && y0 == y1)
                break;

            int e2 = 2 * error;

            if (e2 >= dy)
            {
                error += dy;
                x0 += sx;
            }

            if (e2 <= dx)
            {
                error += dx;
                y0 += sy;
            }
        }
    }

    static void draw_circle(
        uint8_t *image,
        int centre_x,
        int centre_y,
        int radius,
        bool black)
    {
        int x = radius;
        int y = 0;
        int error = 0;

        while (x >= y)
        {
            epaper::set_pixel(image, centre_x + x, centre_y + y, black);
            epaper::set_pixel(image, centre_x + y, centre_y + x, black);
            epaper::set_pixel(image, centre_x - y, centre_y + x, black);
            epaper::set_pixel(image, centre_x - x, centre_y + y, black);

            epaper::set_pixel(image, centre_x - x, centre_y - y, black);
            epaper::set_pixel(image, centre_x - y, centre_y - x, black);
            epaper::set_pixel(image, centre_x + y, centre_y - x, black);
            epaper::set_pixel(image, centre_x + x, centre_y - y, black);

            y++;

            if (error <= 0)
            {
                error += 2 * y + 1;
            }

            if (error > 0)
            {
                x--;
                error -= 2 * x + 1;
            }
        }
    }

    void draw_degree_symbol(
        uint8_t *image,
        int x,
        int y)
    {
        epaper::draw_rect(
            image,
            x,
            y,
            6,
            6,
            true,
            false);
    }

    // ---------------------------------------------------------
    // Date helpers
    // ---------------------------------------------------------

    const char *weekday_name(uint8_t weekday)
    {
        switch (weekday)
        {
        case 0x01:
            return "SUN";
        case 0x02:
            return "MON";
        case 0x04:
            return "TUE";
        case 0x08:
            return "WED";
        case 0x10:
            return "THU";
        case 0x20:
            return "FRI";
        case 0x40:
            return "SAT";
        default:
            return "---";
        }
    }

    const char *month_name(uint8_t month)
    {
        static const char *months[] =
            {
                "---",
                "JAN",
                "FEB",
                "MAR",
                "APR",
                "MAY",
                "JUN",
                "JUL",
                "AUG",
                "SEP",
                "OCT",
                "NOV",
                "DEC"};

        if (month < 1 || month > 12)
        {
            return "---";
        }

        return months[month];
    }

    // ---------------------------------------------------------
    // Build entire clock screen
    // ---------------------------------------------------------

    void build_vertical_screen(
        std::uint8_t *image,
        const DateTime &time,
        const EnvironmentData &environment,
        const EventData &event)
    {
        epaper::set_rotation(epaper::Rotation::Portrait);
        epaper::buffer_clear(image, false);

        char date_text[32];

        std::snprintf(
            date_text,
            sizeof(date_text),
            "%s %u %s",
            weekday_name(time.weekday),
            static_cast<unsigned>(time.day),
            month_name(time.month));

        // Approximate centring:
        // 6 pixels per character * scale 2.
        const int date_length =
            static_cast<int>(std::strlen(date_text));

        const int date_width =
            date_length * 6 * 2;

        const int date_x =
            (epaper::WIDTH - date_width) / 2;

        epaper::draw_text(
            image,
            date_x,
            22,
            date_text,
            2);

        draw_large_time(
            image,
            time.hour,
            time.minute);

        draw_period(
            image,
            time.hour,
            192,
            215);

        epaper::draw_rect(
            image,
            16,
            270,
            epaper::WIDTH - 32,
            1,
            true,
            true);

        // Divider
        epaper::draw_rect(image, 16, 245, epaper::WIDTH - 32, 1, true, true);

        // Temperature Labels
        epaper::draw_text(
            image,
            28,
            292,
            "TEMP",
            2);

        // Humidity Labels
        epaper::draw_text(
            image,
            132,
            292,
            "HUMIDITY",
            2);

        // Environmental values
        if (environment.valid)
        {
            draw_temperature_value(
                image,
                14,
                282,
                environment.temperature_c);

            draw_humidity_value(
                image,
                145,
                282,
                environment.humidity_percent);
        }
        else
        {
            epaper::draw_text(
                image,
                20,
                287,
                "--.- C",
                2);

            epaper::draw_text(
                image,
                155,
                287,
                "--",
                2);

            draw_percent_symbol(
                image,
                195,
                285);

            // Event section
            epaper::draw_rect(image, 16, 335, epaper::WIDTH - 32, 1, true, true);
            epaper::draw_text(image, 18, 347, "NEXT EVENT", 2);
            if (event.valid)
            {
                char event_name[19];
                std::snprintf(event_name, sizeof(event_name), "%.18s", event.name.c_str());
                epaper::draw_text(image, 18, 370, event_name, 1);
                char event_details[40];
                std::snprintf(event_details, sizeof(event_details), "%s %s", event.date.c_str(), event.time.c_str());
                epaper::draw_text(image, 18, 392, event_details, 1);
            }
            else
            {
                epaper::draw_text(
                    image,
                    18,
                    370,
                    "NO UPCOMING EVENTS",
                    1);
            }
        }
    }

    void build_horizontal_screen(
        std::uint8_t *image,
        const DateTime &time,
        const EnvironmentData &environment,
        const EventData &event)
    {
        epaper::set_rotation(epaper::Rotation::Landscape);
        epaper::buffer_clear(image, false);

        constexpr int SCREEN_WIDTH = 416;

        char date_text[32];

        std::snprintf(
            date_text,
            sizeof(date_text),
            "%s %u %s",
            weekday_name(time.weekday),
            static_cast<unsigned>(time.day),
            month_name(time.month));

        constexpr int DATE_SCALE = 2;

        const int date_length = static_cast<int>(std::strlen(date_text));

        const int date_width =
            date_length *
            6 *
            DATE_SCALE;

        const int date_x = (SCREEN_WIDTH - date_width) / 2;

        epaper::draw_text(
            image,
            date_x,
            12,
            date_text,
            DATE_SCALE);

            // Left side date
            epaper::draw_text(
                image,
                15,
                52,
                "NEXT EVENT",
                2);

        if (event.valid)
        {
            char event_name[17];

            std::snprintf(event_name, sizeof(event_name), "%.16s", event.name.c_str());
            epaper::draw_text(image, 275, 82, event_name, 2);
            epaper::draw_text(image, 275, 115, event.date.c_str(), 1);
            epaper::draw_text( image, 275, 138, event.time.c_str(), 2);
        }
        else
        {
            epaper::draw_text(
                image,
                275,
                85,
                "NO EVENTS",
                2
            );
        }

        draw_large_time_horizontal(
            image,
            time.hour,
            time.minute);

        draw_period(
            image,
            time.hour,
            215,
            145
        );

        epaper::draw_rect(
            image,
            24,
            170,
            368,
            1,
            true,
            true);

        // Temp label
        epaper::draw_text(
            image,
            55,
            178,
            "TEMP",
            2);

        // Humidity label
        epaper::draw_text(
            image,
            270,
            178,
            "HUMIDITY",
            2);

        if (environment.valid)
        {
            draw_temperature_value(
                image,
                40,
                196,
                environment.temperature_c);

            draw_humidity_value(
                image,
                285,
                196,
                environment.humidity_percent);
        }
        else
        {
            epaper::draw_text(
                image,
                55,
                213,
                "--.- C",
                2);

            epaper::draw_text(
                image,
                290,
                213,
                "--",
                2);

            draw_percent_symbol(
                image,
                335,
                211);
        }
    }
}