#include "board.h"
#include <cstdio>
#include <cstdint>

#define SW1 2
#define SW2 3
#define SW3 4
#define BUZZER 10

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "WS2812.pio.h"
#include "drivers/logging/logging.h"
#include "drivers/WS2812/leds.h"
#include "drivers/LIS3DH/lis3dh.h"
#include "drivers/microphone/microphone.h"
#include "drivers/epaper/epaper_3in7.h"
#include "drivers/SEN0546/temp_sensor.h"
#include "drivers/rtc/rtc.h"
#include "drivers/app/clock_app.h"
#include "drivers/MicroSD/MicroSD.h"
#include "time_adjust.h"
#include "drivers/alarm/set_alarm.h"
#include "drivers/buzzer/buzzer.h"

#include "board.h"
#include <cstdio>
#include <cstdint>

volatile bool button_pressed_1 = false;
volatile bool button_pressed_2 = false;
volatile bool button_pressed_3 = false;

// Button callback function
void button_callback(uint pin, uint32_t events) {
    switch (pin) {
        case SW1: button_pressed_1 = true; break;
        case SW2: button_pressed_2 = true; break;
        case SW3: button_pressed_3 = true; break;
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    // Initialise switch 1
    gpio_init(SW1);
    gpio_set_dir(SW1, GPIO_IN);
    gpio_pull_up(SW1);
    gpio_set_irq_enabled_with_callback(SW1, GPIO_IRQ_EDGE_RISE, true, button_callback);

    //Initialise switch 2
    gpio_init(SW2);
    gpio_set_dir(SW2, GPIO_IN);
    gpio_pull_up(SW2);
    gpio_set_irq_enabled_with_callback(SW2, GPIO_IRQ_EDGE_RISE, true, button_callback);

    // Initialise switch 3
    gpio_init(SW3);
    gpio_set_dir(SW3, GPIO_IN);
    gpio_pull_up(SW3);
    gpio_set_irq_enabled_with_callback(SW3, GPIO_IRQ_EDGE_RISE, true, button_callback);

    // Initialise I2C bus
    i2c_init(board::I2C_PORT, board::I2C_BAUD_RATE);
    gpio_set_function(board::I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(board::I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(board::I2C_SDA_PIN);
    gpio_pull_up(board::I2C_SCL_PIN);

    // Initialise RTC
    INS5699S rtc(board::I2C_PORT);
    bool SET_RTC_ON_BOOT = true; // Keep true while RTC has no backup power.

    if (!rtc.initialise()) {
        printf("ERROR: RTC initialisation failed\n");
    }

    if (SET_RTC_ON_BOOT) {
        DateTime initial_time{};

        initial_time.year = 2026;
        initial_time.month = 8;
        initial_time.day = 13;
        initial_time.weekday = 0x10; // Thursday

        initial_time.hour = 13;
        initial_time.minute = 0;
        initial_time.second = 0;

        initial_time.valid = true;

        if (rtc.set_datetime(initial_time)) {
            printf("RTC time set successfully\n");
        } else { 
            printf("ERROR: RTC time setting failed\n");
        }
    }

    // Initialise Alarm
    SetAlarm alarm(rtc, button_pressed_1, button_pressed_2, button_pressed_3);
    bool alarm_on = false;

    // Initialise Buzzer
    Buzzer buzzer(BUZZER, button_pressed_2);

    // Initialise temperature and humidity sensor
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

    // Initialise SD card
    MicroSD sd(
        board::SD_DAT3_PIN,
        board::SD_CLK_PIN,
        board::SD_CMD_PIN,
        board::SD_DAT0_PIN);

    const bool sd_ready = sd.init();

    if (sd_ready) {
        printf("SD card initialised successfully\n");
    } else {
        printf("ERROR: SD card initialisation failed\n");
    }

    app::EventData next_event{};

    if (sd_ready) {
        const std::vector<std::string> fields = sd.get_next_event("events.txt");

        if (fields.size() >= 3) {
            next_event.name = fields[0];
            next_event.date = fields[1];
            next_event.time = fields[2];
            next_event.valid = true;
        }
    }

    // Initialise accelerometer for orientation
    gpio_init(board::ORIENTATION_PIN);
    gpio_set_dir(board::ORIENTATION_PIN, GPIO_IN);
    gpio_pull_down(board::ORIENTATION_PIN);
    printf("Orientation input initialised on GPIO %u\n", static_cast<unsigned>(board::ORIENTATION_PIN));

    // Initialise display pins
    const epaper::Pins display_pins {
        .sck = board::EPD_SCK_PIN,
        .mosi = board::EPD_MOSI_PIN,
        .cs = board::EPD_CS_PIN,
        .dc = board::EPD_DC_PIN,
        .reset = board::EPD_RST_PIN,
        .busy = board::EPD_BUSY_PIN};

    epaper::Epaper3in7 display(
        board::EPD_SPI,
        display_pins);

    if (!display.init()) {
        printf("ERROR: Display initialisation failed\n");
        while (true) {
            sleep_ms(1000);
        }
    }

    static uint8_t image[epaper::BUFFER_SIZE];

    int previous_minute = -1;
    int previous_mode = -1;

    absolute_time_t next_environment_read = make_timeout_time_ms(30000);

    while (true) {
        if (button_pressed_1) {
        button_pressed_1 = false;
        TimeAdjust adjuster(rtc, button_pressed_1, button_pressed_2, button_pressed_3);
        adjuster.run();
        SET_RTC_ON_BOOT = false;
        }

        if (button_pressed_3) {
            button_pressed_3 = false;
            alarm.set_alarm();
            alarm_on = true;
        }

        const DateTime now = rtc.read_datetime(); // get current time from RTC

        const int mode = gpio_get(board::ORIENTATION_PIN); // Check clock orientation

        if (absolute_time_diff_us(get_absolute_time(), next_environment_read) <= 0) {
            const EnvironmentData reading = sensor.read();
            if (reading.valid) {
                latest_environment = reading;
            }
            next_environment_read = make_timeout_time_ms(30000);
        }

        const bool minute_changed = now.valid && static_cast<int>(now.minute) != previous_minute;
        const bool orientation_changed = mode != previous_mode;

        if (now.valid && orientation_changed) {
            printf("Orientation changed -> %s\n", mode == 1 ? "VERTICAL" : "HORIZONTAL");

            if (mode == 1) {
                app::build_vertical_screen(
                    image,
                    now,
                    latest_environment,
                    next_event);
            } else {
                app::build_horizontal_screen(
                    image,
                    now,
                    latest_environment,
                    next_event);
            }

            if (!display.init()) {
                printf("ERROR: Display reinitialisation failed\n");
            } else if (display.display(image)) {
                printf("Orientation display refresh complete\n");
            } else {
                printf("ERROR: Orientation refresh failed\n");
            }

            previous_mode = mode;
            previous_minute = static_cast<int>(now.minute);

        } else if (minute_changed) {
            printf("\nMinute changed -> %02u:%02u\n",
                static_cast<unsigned>(now.hour),
                static_cast<unsigned>(now.minute));

            if (mode == 1) {
                app::build_vertical_screen(
                    image,
                    now,
                    latest_environment,
                    next_event);
            } else {
                app::build_horizontal_screen(
                    image,
                    now,
                    latest_environment,
                    next_event);
            }

            if (display.display(image)) {
                printf("Minute display refresh complete\n");
            } else {
                printf("ERROR: Minute refresh failed\n");
            }

            if (sd_ready) {
                char temperature_buffer[16];
                char humidity_buffer[16];

                if (latest_environment.valid) {
                    snprintf(
                        temperature_buffer,
                        sizeof(temperature_buffer),"%.1fC",
                        static_cast<double>(latest_environment.temperature_c));

                    snprintf(
                        humidity_buffer,
                        sizeof(humidity_buffer),"%.0f%%",
                        static_cast<double>(latest_environment.humidity_percent));
                } else {
                    snprintf(temperature_buffer, sizeof(temperature_buffer), "--.-C");
                    snprintf(humidity_buffer, sizeof(humidity_buffer), "--%%");
                }

                sd.writeData(DATA_FILE, temperature_buffer, humidity_buffer,
                    std::to_string(now.year).c_str(),
                    std::to_string(now.month).c_str(),
                    std::to_string(now.day).c_str(),
                    std::to_string(now.hour).c_str(),
                    std::to_string(static_cast<unsigned>(now.minute)).c_str());

                printf("SD data written: %s | %s\n", temperature_buffer, humidity_buffer);
                std::vector<std::string>next_event =sd.get_next_event("EVENTS.txt");

                // Protect against indexing an incomplete/empty event.
                if (next_event.size() >= 3) {
                    printf("Next event: ");

                    for (size_t i = 0; i < next_event.size(); ++i) {
                        printf("%s ", next_event[i].c_str());
                    }

                    printf("\n");

                    char date_buffer[11];
                    char time_buffer[6];

                    snprintf(
                        date_buffer,
                        sizeof(date_buffer),
                        "%02u/%02u/%04u",
                        static_cast<unsigned>(now.day),
                        static_cast<unsigned>(now.month),
                        static_cast<unsigned>(now.year));

                    snprintf(
                        time_buffer,
                        sizeof(time_buffer),
                        "%02u:%02u",
                        static_cast<unsigned>(now.hour),
                        static_cast<unsigned>(now.minute));

                    if (next_event[1] == date_buffer && next_event[2] == time_buffer) {
                        sd.deleteLine("EVENTS.txt");
                        printf("Completed event removed from EVENTS.txt\n");
                    }
                }
            }
            previous_minute = static_cast<int>(now.minute);
            previous_mode = mode;
            if (alarm_on) {
                if (alarm.check_alarm() && alarm.alarm_flag == false) {
                    alarm.alarm_flag = true;
                    button_pressed_2 = false;
                    buzzer.alarm();
                }
            }
        }
        sleep_ms(50);
    }
}