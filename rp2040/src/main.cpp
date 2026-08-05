#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "WS2812.pio.h" //This header file gets produced during compilation from the WS2812.pio file
#include "drivers/logging/logging.h"
#include "drivers/WS2812/leds.h"
#include "drivers/LIS3DH/lis3dh.h"
#include "drivers/microphone/microphone.h"
#include "drivers/epaper/epaper_3in7.h"
#include "drivers/SEN0546/temp_sensor.h"
#include "drivers/rtc/rtc.h"
#include "drivers/app/clock_app.h"

#include "board.h"
#include <cstdio>
#include <cstdint>

#include <cstdio>
#include <cstdint>

#include "pico/stdlib.h"

#include "board.h"
#include "drivers/epaper/epaper_3in7.h"

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("\n");
    printf("Smart Desk Clock - E-paper diagnostic test\n");
    printf("------------------------------------------\n");

    printf("1: Creating display pin configuration\n");

    epaper::Pins display_pins{
        .sck = board::EPD_SCK_PIN,
        .mosi = board::EPD_MOSI_PIN,
        .cs = board::EPD_CS_PIN,
        .dc = board::EPD_DC_PIN,
        .reset = board::EPD_RST_PIN,
        .busy = board::EPD_BUSY_PIN
    };

    printf("2: Constructing display object\n");

    epaper::Epaper3in7 display(
        board::EPD_SPI,
        display_pins
    );

    printf("3: Calling display.init()\n");

    const bool init_ok = display.init();

    printf("4: display.init() returned %d\n", init_ok);

    if (!init_ok)
    {
        printf("ERROR: Display initialisation failed\n");

        while (true)
        {
            printf("Display init failed - program still running\n");
            sleep_ms(1000);
        }
    }

    printf("5: Creating image buffer\n");

    static uint8_t image[epaper::BUFFER_SIZE];

    epaper::buffer_clear(image, false);

    printf("6: Drawing test graphics\n");

    epaper::draw_rect(
        image,
        10,
        20,
        220,
        140,
        true,
        false
    );

    epaper::draw_text(
        image,
        20,
        40,
        "SMART DESK CLOCK",
        3
    );

    epaper::draw_text(
        image,
        20,
        100,
        "DISPLAY TEST",
        3
    );

    printf("7: Calling display.display()\n");

    const bool display_ok = display.display(image);

    printf("8: display.display() returned %d\n", display_ok);

    if (!display_ok)
    {
        printf("ERROR: Display refresh failed\n");

        while (true)
        {
            printf("Display refresh failed - program still running\n");
            sleep_ms(1000);
        }
    }

    printf("9: Display test completed successfully\n");

    while (true)
    {
        printf("Program still running\n");
        sleep_ms(1000);
    }
}