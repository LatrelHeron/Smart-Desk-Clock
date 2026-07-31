#pragma once

#include <stdint.h>

void leds_init();

void leds_set(int led,
              uint8_t red,
              uint8_t green,
              uint8_t blue);

void leds_show();

void leds_clear();

void leds_set_range(int start,
                    int end,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue);

enum class LedMode
{
    OFF,
    BOOTING,
    OK,
    SENSOR_ERROR
};

void leds_set_mode(LedMode mode);