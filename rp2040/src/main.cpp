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


int main() {
  
}

