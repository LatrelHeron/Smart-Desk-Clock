#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "WS2812.pio.h" // This header file gets produced during compilation from the WS2812.pio file
#include "drivers/logging/logging.h"
#include "drivers/WS2812/leds.h"
#include "drivers/LIS3DH/lis3dh.h"
#include "drivers/microphone/microphone.h"
#include "drivers/epaper/epaper_3in7.h"
#include "board.h"
#include <cstdio>
#include <cstdint>
/*
int main()
{
    stdio_init_all();
    sleep_ms(2000);

    leds_init();
    leds_clear();

    // Wiring chosen from your available-header pin table:
    // SCL/SCK -> GP10
    // SDA/MOSI -> GP11
    // CS -> GP9
    // D/C -> GP27
    // RES -> GP28
    // BUSY -> GP8
    constexpr epaper::Pins EPD_PINS{
        .sck = 10,
        .mosi = 11,
        .cs = 9,
        .dc = 27,
        .reset = 28,
        .busy = 8,
    };

    epaper::Epaper3in7 display(spi1, EPD_PINS);

    printf("Starting WeAct 3.7-inch e-paper test...\n");

    if (!display.init())
    {
        printf("E-paper init failed or BUSY timed out.\n");
        while (true)
        {
            sleep_ms(1000);
        }
    }

    static uint8_t image[epaper::BUFFER_SIZE];
    epaper::buffer_clear(image, false);

    epaper::draw_rect(image, 8, 8, 224, 400, true, false);
    epaper::draw_text(image, 24, 35, "CC3501", 4);
    epaper::draw_text(image, 24, 95, "E-DISPLAY", 3);
    epaper::draw_text(image, 24, 135, "TEST", 3);

    epaper::draw_rect(image, 24, 200, 192, 38, true, false);
    epaper::draw_rect(image, 30, 206, 120, 26, true, true);

    epaper::draw_text(image, 24, 275, "DISPLAY", 2);
    epaper::draw_text(image, 24, 300, "WORKING", 2);

    if (display.display(image))
    {
        printf("Display refresh completed.\n");
    }
    else
    {
        printf("Display refresh failed or BUSY timed out.\n");
    }

    // E-paper keeps the image without continuous power.
    display.sleep();
    printf("Display put into deep sleep.\n");

    while (true)
    {
        sleep_ms(1000);
    }
}
*/

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    leds_init();
    leds_clear();

    constexpr epaper::Pins EPD_PINS{
        .sck = 10,
        .mosi = 11,
        .cs = 9,
        .dc = 27,
        .reset = 28,
        .busy = 8,
    };

    epaper::Epaper3in7 display(spi1, EPD_PINS);

    printf("Starting simulated e-paper clock...\n");

    if (!display.init())
    {
        printf("E-paper initialisation failed or BUSY timed out.\n");

        while (true)
        {
            sleep_ms(1000);
        }
    }

    static uint8_t image[epaper::BUFFER_SIZE];

    int hour = 12;
    int minute = 58;

    // Run 10 simulated clock updates.
    for (int update = 0; update < 10; update++)
    {
        char time_text[8];
        char transition_text[32];

        // Format time as HH:MM.
        snprintf(
            time_text,
            sizeof(time_text),
            "%02d:%02d",
            hour,
            minute
        );

        snprintf(
            transition_text,
            sizeof(transition_text),
            "UPDATE %d OF 10",
            update + 1
        );

        // Clear the previous software image.
        epaper::buffer_clear(image, false);

        // Outer clock border.
        epaper::draw_rect(
            image,
            8,
            8,
            224,
            400,
            true,
            false
        );

        epaper::draw_text(
            image,
            28,
            35,
            "SMART CLOCK",
            2
        );

        epaper::draw_text(
            image,
            25,
            115,
            time_text,
            5
        );

        epaper::draw_text(
            image,
            38,
            205,
            "THURSDAY",
            2
        );

        epaper::draw_text(
            image,
            28,
            240,
            "30 JULY 2026",
            2
        );

        epaper::draw_text(
            image,
            35,
            315,
            transition_text,
            1
        );

        // Progress bar showing that the display is actively updating.
        epaper::draw_rect(
            image,
            25,
            350,
            190,
            24,
            true,
            false
        );

        int progress_width = (update + 1) * 18;

        epaper::draw_rect(
            image,
            30,
            355,
            progress_width,
            14,
            true,
            true
        );

        printf("Displaying simulated time: %s\n", time_text);

        if (!display.display(image))
        {
            printf("Display refresh failed or BUSY timed out.\n");
            break;
        }

        printf("Display refresh completed.\n");

        // Simulate one minute passing every three seconds.
        minute++;

        if (minute >= 60)
        {
            minute = 0;
            hour++;

            if (hour >= 24)
            {
                hour = 0;
            }
        }

        sleep_ms(3000);
    }

    // Final screen after the demonstration.
    epaper::buffer_clear(image, false);

    epaper::draw_rect(
        image,
        8,
        8,
        224,
        400,
        true,
        false
    );

    epaper::draw_text(
        image,
        30,
        130,
        "CLOCK DEMO",
        3
    );

    epaper::draw_text(
        image,
        55,
        190,
        "COMPLETE",
        3
    );

    display.display(image);

    // Only put the display to sleep after all updates are finished.
    display.sleep();

    printf("Clock demonstration complete.\n");
    printf("Display placed into deep sleep.\n");

    while (true)
    {
        sleep_ms(1000);
    }
}
