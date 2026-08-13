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
#include "time_adjust.h"
#include "drivers/alarm/set_alarm.h"
#include "drivers/buzzer/buzzer.h"

#include "board.h"
#include <cstdio>
#include <cstdint>

#define SW1 2
#define SW2 3
#define SW3 4
#define BUZZER 10

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

int main() {
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

    printf("\nButtons and alarm test\n");


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

    // Alarm
    SetAlarm alarm(rtc, button_pressed_1, button_pressed_2, button_pressed_3);

    // Buzzer
    Buzzer buzzer(BUZZER, button_pressed_2);

    if (!rtc.initialise())
    {
       printf("ERROR: RTC initialisation failed\n");
    }

    constexpr bool SET_RTC_ON_BOOT = true;
    bool alarm_on = false;

    if (SET_RTC_ON_BOOT)
    {
       DateTime initial_time{};

       initial_time.year = 2026;
       initial_time.month = 8;
       initial_time.day = 10;
       initial_time.weekday = 0x08; // Wednesday
       initial_time.hour = 18;
       initial_time.minute = 00;
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
   

    printf("All available devices initialised\n");

    int previous_minute = -1;

   absolute_time_t next_environment_read = make_timeout_time_ms(30000);

   while (true)
   {
    if (button_pressed_1) {
        button_pressed_1 = false;
        TimeAdjust adjuster(rtc, button_pressed_1, button_pressed_2, button_pressed_3);
        adjuster.run();
        }

    if (button_pressed_3) {
        button_pressed_3 = false;
        alarm.set_alarm();
        alarm_on = true;
    }

    const DateTime now = rtc.read_datetime();
        if (now.valid &&
           static_cast<int>(now.minute) != previous_minute)
        {
            printf(
               "Time: %02u:%02u  Date: %02u/%02u/%04u\n",
               static_cast<unsigned>(now.hour),
               static_cast<unsigned>(now.minute),
               static_cast<unsigned>(now.day),
               static_cast<unsigned>(now.month),
               static_cast<unsigned>(now.year)
            );
        }
        previous_minute = now.minute;
    if (alarm_on) {
        if (alarm.check_alarm() && alarm.alarm_flag == false) {
            alarm.alarm_flag = true;
            button_pressed_2 = false;
            buzzer.alarm();
        }
    }
    }
}

