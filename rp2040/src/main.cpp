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
#include "drivers/SEN0546/temp_sensor.h"

#include "board.h"
#include <cstdio>
#include <cstdint>

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    leds_init();

    while (true)
    {
        // Red
        leds_set(0, 30, 0, 0);
        leds_show();
        sleep_ms(1000);

        // Green
        leds_set(0, 0, 30, 0);
        leds_show();
        sleep_ms(1000);

        // Blue
        leds_set(0, 0, 0, 30);
        leds_show();
        sleep_ms(1000);

        // Off
        leds_clear();
        leds_show();
        sleep_ms(1000);
    }

    printf("\n");
    printf("Smart Desk Clock - SEN0546 test\n");
    printf("--------------------------------\n");

    i2c_init(
        board::I2C_PORT,
        board::I2C_BAUD_RATE);

    gpio_set_function(
        board::I2C_SDA_PIN,
        GPIO_FUNC_I2C);

    gpio_set_function(
        board::I2C_SCL_PIN,
        GPIO_FUNC_I2C);

    /*
    These enable the RP2040's weak internal pull-ups.

    The SEN0546 module reportedly already has external
    4.7 kOhm pull-ups, so these are not the primary pull-ups.
    */
    gpio_pull_up(board::I2C_SDA_PIN);
    gpio_pull_up(board::I2C_SCL_PIN);

    SEN0546 sensor(board::I2C_PORT);

    if (!sensor.initialise())
    {
        printf("Sensor initialisation failed\n");

        while (true)
        {
            sleep_ms(1000);
        }
    }

    printf(
        "Sensor ready: %s\n",
        sensor.sensor_name());

    while (true)
    {
        const EnvironmentData environment =
            sensor.read();

        if (environment.valid)
        {
            printf(
                "Temperature: %.2f C | Humidity: %.2f %%RH\n",
                environment.temperature_c,
                environment.humidity_percent);
        }
        else
        {
            printf("Sensor reading failed\n");
        }

        sleep_ms(1000);
    }
}