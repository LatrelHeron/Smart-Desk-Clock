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
#include "drivers/MicroSD/MicroSD.h"

#include "board.h"
#include <cstdio>
#include <cstdint>


int main() {
    stdio_init_all();
    sleep_ms(2000);

    printf("\nSmart Desk Clock SD card test\n");
    MicroSD sd(
        board::SD_DAT3_PIN,
        board::SD_CLK_PIN,
        board::SD_CMD_PIN,
        board::SD_DAT0_PIN
    );

    if (!sd.init()) {
        printf("ERROR: SD card initialisation failed\n");
    } else {
        if (sd.appendText(DATA_FILE, "Hello, world!\n")) {
            printf("Text appended to file successfully\n");
        } else {
            printf("ERROR: Failed to append text to file\n");
        }

        std::string output;
        if (sd.readText(DATA_FILE, output)) {
            printf("Read from file: %s", output.c_str());
        } else {
            printf("ERROR: Failed to read from file\n");
        }
    }
}
/*
int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("\nSmart Desk Clock integration test\n");

    // Initialise the shared I2C bus.
    i2c_init(
        board::I2C_PORT,
        board::I2C_BAUD_RATE);

    gpio_set_function(
        board::I2C_SDA_PIN,
        GPIO_FUNC_I2C);

    gpio_set_function(
        board::I2C_SCL_PIN,
        GPIO_FUNC_I2C);

    gpio_pull_up(board::I2C_SDA_PIN);
    gpio_pull_up(board::I2C_SCL_PIN);

    // RTC.
    INS5699S rtc(board::I2C_PORT);

    if (!rtc.initialise())
    {
        printf("ERROR: RTC initialisation failed\n");
    }

    constexpr bool SET_RTC_ON_BOOT = true;

    if (SET_RTC_ON_BOOT)
    {
        DateTime initial_time{};

        initial_time.year = 2026;
        initial_time.month = 8;
        initial_time.day = 5;
        initial_time.weekday = 0x08; // Wednesday
        initial_time.hour = 19;
        initial_time.minute = 42;
        initial_time.second = 0;
        initial_time.valid = true;

        if (rtc.set_datetime(initial_time))
        {
            printf("RTC time set successfully\n");
        }
        else
        {
            printf("ERROR: RTC time setting failed\n");
        }
    }
    // Temperature/humidity sensor.
    SEN0546 sensor(board::I2C_PORT);
    EnvironmentData latest_environment{};

    if (sensor.initialise())
    {
        latest_environment = sensor.read();
    }
    else
    {
        printf("ERROR: Temperature sensor initialisation failed\n");
    }

    // E-paper pin mapping confirmed on the PCB.
    const epaper::Pins display_pins{
        .sck = board::EPD_SCK_PIN,
        .mosi = board::EPD_MOSI_PIN,
        .cs = board::EPD_CS_PIN,
        .dc = board::EPD_DC_PIN,
        .reset = board::EPD_RST_PIN,
        .busy = board::EPD_BUSY_PIN};

    epaper::Epaper3in7 display(
        board::EPD_SPI,
        display_pins);

    if (!display.init())
    {
        printf("ERROR: Display initialisation failed\n");

        while (true)
        {
            sleep_ms(1000);
        }
    }

    printf("All available devices initialised\n");

    static uint8_t image[epaper::BUFFER_SIZE];

    int previous_minute = -1;

    absolute_time_t next_environment_read = make_timeout_time_ms(30000);

    while (true)
    {
        const DateTime now = rtc.read_datetime();

        // Read the sensor independently every 30 seconds.
        if (absolute_time_diff_us(
                get_absolute_time(),
                next_environment_read) <= 0)
        {
            const EnvironmentData new_environment = sensor.read();

            if (new_environment.valid)
            {
                latest_environment = new_environment;
            }

            next_environment_read = make_timeout_time_ms(30000);
        }

        // Refresh the e-paper only when the minute changes.
        if (now.valid &&
            static_cast<int>(now.minute) != previous_minute)
        {
            printf(
                "Time: %02u:%02u  Date: %02u/%02u/%04u\n",
                static_cast<unsigned>(now.hour),
                static_cast<unsigned>(now.minute),
                static_cast<unsigned>(now.day),
                static_cast<unsigned>(now.month),
                static_cast<unsigned>(now.year));

            if (latest_environment.valid)
            {
                printf(
                    "Temperature: %.1f C  Humidity: %.1f %%\n",
                    static_cast<double>(
                        latest_environment.temperature_c),
                    static_cast<double>(
                        latest_environment.humidity_percent));
            }
            app::build_clock_screen(
                image,
                now,
                latest_environment);
            if (display.display_partial(
                    image,
                    40,
                    80,
                    176,
                    60))
            {
                printf("Display updated successfully\n");
            }
            else
            {
                printf("ERROR: Display update failed\n");
            }

            previous_minute = now.minute;
        }

        // RTC can still be checked frequently without refreshing the display.
        sleep_ms(250);
    }
}
    */