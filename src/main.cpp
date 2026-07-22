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
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include <cstdio>

int main() {
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

    if (!display.init()) {
        printf("E-paper init failed or BUSY timed out.\n");
        while (true) {
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

    if (display.display(image)) {
        printf("Display refresh completed.\n");
    } else {
        printf("Display refresh failed or BUSY timed out.\n");
    }

    // E-paper keeps the image without continuous power.
    display.sleep();
    printf("Display put into deep sleep.\n");

    while (true) {
        sleep_ms(1000);
    }
}
