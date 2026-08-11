#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "WS2812.pio.h" //This header file gets produced during compilation from the WS2812.pio file
#include "drivers/LIS3DH/lis3dh.h"
#include "drivers/WS2812/leds.h"

#include "board.h"

// -----------------------------------------------------
// Orientation output to Smart Desk Clock PCB
//
// Dev board GPIO12 ---> Smart Clock PCB GPIO26
//
// 0 = Horizontal
// 1 = Vertical
// -----------------------------------------------------

constexpr uint ORIENTATION_OUT_PIN = 12;

// -----------------------------------------------------
// LED mode indication
//
// BLUE  = Horizontal
// GREEN = Vertical
// RED   = Accelerometer error
// -----------------------------------------------------

void set_all_leds_horizontal()
{
    leds_set_range(
        0,
        LED_COUNT - 1,
        0,   // Red
        0,   // Green
        35); // Blue

    leds_show();
}

void set_all_leds_vertical()
{
    leds_set_range(
        0,
        LED_COUNT - 1,
        0,  // Red
        35, // Green
        0); // Blue

    leds_show();
}

void set_all_leds_error()
{
    leds_set_range(
        0,
        LED_COUNT - 1,
        35, // Red
        0,  // Green
        0); // Blue

    leds_show();
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("\nLIS3DH Orientation Transmitter\n");

    // -----------------------------------------------------
    // Initialise LED array
    // -----------------------------------------------------

    leds_init();

    // Default visual state is horizontal.
    set_all_leds_horizontal();

    // -----------------------------------------------------
    // Initialise orientation output GPIO
    //
    // This GPIO sends the current mode to the
    // Smart Desk Clock PCB.
    // -----------------------------------------------------

    gpio_init(ORIENTATION_OUT_PIN);

    gpio_set_dir(
        ORIENTATION_OUT_PIN,
        GPIO_OUT);

    // Default to horizontal.
    gpio_put(
        ORIENTATION_OUT_PIN,
        0);

    // -----------------------------------------------------
    // Initialise accelerometer
    // -----------------------------------------------------

    if (!lis3dh_init())
    {
        printf(
            "ERROR: LIS3DH initialisation failed\n");

        // Red LEDs indicate sensor failure.
        set_all_leds_error();

        while (true)
        {
            sleep_ms(1000);
        }
    }

    printf(
        "LIS3DH initialised successfully\n");

    // -----------------------------------------------------
    // Current orientation
    //
    // false = horizontal
    // true  = vertical
    // -----------------------------------------------------

    bool vertical = false;

    bool previous_vertical = vertical;

    // -----------------------------------------------------
    // Main application loop
    // -----------------------------------------------------

    while (true)
    {
        lis3dh_g_t accel =
            lis3dh_read_g();

        // -------------------------------------------------
        // Convert X and Y acceleration to absolute values.
        // -------------------------------------------------

        const float abs_x =
            accel.x < 0.0f
                ? -accel.x
                : accel.x;

        const float abs_y =
            accel.y < 0.0f
                ? -accel.y
                : accel.y;

        // -------------------------------------------------
        // Orientation detection
        //
        // Y axis dominant -> Vertical
        // X axis dominant -> Horizontal
        //
        // 0.70 g threshold prevents small movements and
        // vibration from constantly switching modes.
        // -------------------------------------------------

        if (abs_y > 0.70f)
        {
            vertical = true;
        }
        else if (abs_x > 0.70f)
        {
            vertical = false;
        }

        // -------------------------------------------------
        // Send orientation mode to Smart Desk Clock PCB
        //
        // Horizontal -> GPIO12 LOW
        // Vertical   -> GPIO12 HIGH
        // -------------------------------------------------

        gpio_put(
            ORIENTATION_OUT_PIN,
            vertical ? 1 : 0);

        // -------------------------------------------------
        // Update LEDs only when orientation changes.
        // -------------------------------------------------

        if (vertical != previous_vertical)
        {
            if (vertical)
            {
                set_all_leds_vertical();

                printf(
                    "Orientation changed: VERTICAL\n");
            }
            else
            {
                set_all_leds_horizontal();

                printf(
                    "Orientation changed: HORIZONTAL\n");
            }

            previous_vertical =
                vertical;
        }

        // -------------------------------------------------
        // Serial debugging
        // -------------------------------------------------

        printf(
            "X: %.2f g  Y: %.2f g  Z: %.2f g | Mode: %d (%s) | GPIO12: %d\n",
            static_cast<double>(accel.x),
            static_cast<double>(accel.y),
            static_cast<double>(accel.z),
            vertical ? 1 : 0,
            vertical
                ? "VERTICAL"
                : "HORIZONTAL",
            gpio_get(ORIENTATION_OUT_PIN));

        sleep_ms(250);
    }
}


