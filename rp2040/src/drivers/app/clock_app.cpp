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

    // ---------------------------------------------------------
    // Small widget icons
    // ---------------------------------------------------------

    void draw_thermometer(
        uint8_t *image,
        int x,
        int y)
    {
        /*
        // First design for thermometer
        // Stem.
        epaper::draw_rect(
            image,
            x + 7,
            y,
            6,
            28,
            true,
            false);

        // Mercury.
        epaper::draw_rect(
            image,
            x + 9,
            y + 10,
            2,
            20,
            true,
            true);

        // Bulb.
        epaper::draw_rect(
            image,
            x + 4,
            y + 27,
            12,
            12,
            true,
            false);

        epaper::draw_rect(
            image,
            x + 7,
            y + 30,
            6,
            6,
            true,
            true);
        */
        // Stem outline
        epaper::draw_rect(image, x + 8, y + 2, 8, 15, true, false);

        // Clear inside of stem
        epaper::draw_rect(image, x + 10, y + 4, 4, 13, false, true);

        // Bulb
        draw_circle(image, x + 12, y + 18, 6, true);

        // Clear centre of bulb
        draw_circle(image, x + 12, y + 18, 3, false);

        // Mercury column
        epaper::draw_rect(image, x + 11, y + 8, 2, 10, true, true);

        // Filled centre of bulb
        draw_circle(image, x + 12, y + 18, 3, true);

        // Small temperature ticks on right
        draw_hline(image, x + 16, y + 6, 3, true);
        draw_hline(image, x + 16, y + 10, 3, true);
        draw_hline(image, x + 16, y + 14, 3, true);
    }

    void draw_water_drop(
        uint8_t *image,
        int x,
        int y)
    {
        /*
        // First design for water drop
        // Simple pixel droplet.
        epaper::draw_rect(image, x + 8, y, 4, 4, true, true);
        epaper::draw_rect(image, x + 6, y + 4, 8, 4, true, true);
        epaper::draw_rect(image, x + 4, y + 8, 12, 4, true, true);
        epaper::draw_rect(image, x + 2, y + 12, 16, 8, true, true);
        epaper::draw_rect(image, x, y + 20, 20, 8, true, true);
        epaper::draw_rect(image, x + 2, y + 28, 16, 6, true, true);
        epaper::draw_rect(image, x + 6, y + 34, 8, 4, true, true);
        */
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

    void draw_percent_symbol(
        uint8_t *image,
        int x,
        int y)
    {
        epaper::draw_rect(
            image,
            x,
            y,
            5,
            5,
            true,
            false);

        epaper::draw_rect(
            image,
            x + 10,
            y + 16,
            5,
            5,
            true,
            false);

        // Diagonal slash.
        for (int i = 0; i < 16; ++i)
        {
            epaper::set_pixel(
                image,
                x + 14 - (i * 10 / 16),
                y + 3 + i,
                true);
        }
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

    void build_home_screen(
        uint8_t *image,
        const DateTime &time,
        const EnvironmentData &environment)
    {
        epaper::buffer_clear(image, false);

        // -----------------------------------------------------
        // Date
        // -----------------------------------------------------

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

        // -----------------------------------------------------
        // Main time
        // -----------------------------------------------------

        draw_large_time(
            image,
            time.hour,
            time.minute);

        // -----------------------------------------------------
        // Widget separator
        // -----------------------------------------------------
        /*
        epaper::draw_rect(
            image,
            8,
            245,
            epaper::WIDTH - 16,
            1,
            true,
            true);

        // Vertical divider.
        epaper::draw_rect(
            image,
            120,
            265,
            1,
            75,
            true,
            true);
        */
        // -----------------------------------------------------
        // Temperature widget
        // -----------------------------------------------------

        draw_thermometer(
            image,
            16,
            280);

        char temperature_text[16];

        if (environment.valid)
        {
            std::snprintf(
                temperature_text,
                sizeof(temperature_text),
                "%.1f",
                static_cast<double>(
                    environment.temperature_c));
        }
        else
        {
            std::snprintf(
                temperature_text,
                sizeof(temperature_text),
                "--.-");
        }

        epaper::draw_text(
            image,
            42,
            288,
            temperature_text,
            3);

        draw_degree_symbol(
            image,
            101,
            283);

        epaper::draw_text(
            image,
            108,
            288,
            "C",
            2);

        // -----------------------------------------------------
        // Humidity widget
        // -----------------------------------------------------

        draw_water_drop(
            image,
            137,
            279);

        char humidity_text[16];

        if (environment.valid)
        {
            std::snprintf(
                humidity_text,
                sizeof(humidity_text),
                "%.0f",
                static_cast<double>(
                    environment.humidity_percent));
        }
        else
        {
            std::snprintf(
                humidity_text,
                sizeof(humidity_text),
                "--");
        }

        epaper::draw_text(
            image,
            165,
            288,
            humidity_text,
            3);

        draw_percent_symbol(
            image,
            210,
            285);

        // Bottom line for some visual structure.
        epaper::draw_rect(
            image,
            8,
            355,
            epaper::WIDTH - 16,
            1,
            true,
            true);
    }
}