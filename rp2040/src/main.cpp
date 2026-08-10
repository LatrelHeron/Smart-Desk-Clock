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

    if (!sd.init()) {
        printf("ERROR: SD card initialisation failed\n");
    } 
    
    int previous_minute = -1;

   absolute_time_t next_environment_read = make_timeout_time_ms(30000);

   while (true)
   {
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
            sd.writeData(DATA_FILE,
                "25C", 
                "55%", 
                std::to_string(now.year).c_str(), 
                std::to_string(now.month).c_str(), 
                std::to_string(now.day).c_str(), 
                std::to_string(now.hour).c_str(), 
                std::to_string(static_cast<unsigned>(now.minute)).c_str());

            std::vector<std::string> next_event = sd.get_next_event("EVENTS.txt");
            for (size_t i = 0; i < next_event.size(); i++) {
                printf("%s ", next_event[i].c_str());
            }

            char date_buffer[11];
            char time_buffer[6];

            snprintf(
                date_buffer,
                sizeof(date_buffer),
                "%02u/%02u/%04u",
                static_cast<unsigned int>(now.day),
                static_cast<unsigned int>(now.month),
                static_cast<unsigned int>(now.year)
            );

            snprintf(
                time_buffer,
                sizeof(time_buffer),
                "%02u:%02u",
                static_cast<unsigned int>(now.hour),
                static_cast<unsigned int>(now.minute)
            );

            if (next_event[1] == date_buffer &&
                next_event[2] == time_buffer)
            {
                sd.deleteLine("EVENTS.txt");
                printf("comparison success");
            }
        }
    previous_minute = now.minute;
    }
}