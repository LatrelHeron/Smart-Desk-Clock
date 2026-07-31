#include "drivers/WS2812/leds.h"

#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "WS2812.pio.h"
#include "board.h"

static uint32_t led_data[LED_COUNT];

static PIO led_pio = pio0;
static uint led_sm = 0;

void leds_init()
{
    uint pio_program_offset = pio_add_program(led_pio, &ws2812_program);
    ws2812_program_init(led_pio, led_sm, pio_program_offset, LED_PIN, 800000, false);

    leds_clear();
    leds_show();
}

void leds_set(int led,
              uint8_t red,
              uint8_t green,
              uint8_t blue)
{
    if (led < 0 || led >= LED_COUNT)
    {
        return;
    }

    led_data[led] = (red << 24) |
                    (green << 16) |
                    (blue << 8);
}

void leds_show()
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        pio_sm_put_blocking(led_pio, led_sm, led_data[i]);
    }
}

void leds_clear()
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        led_data[i] = 0;
    }
}

void leds_set_range(int start,
                    int end,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue)
{
    // Safety checks
    if (start < 0)
    {
        start = 0;
    }

    if (end >= LED_COUNT)
    {
        end = LED_COUNT - 1;
    }

    for (int i = start; i <= end; i++)
    {
        leds_set(i, red, green, blue);
    }
}

void leds_set_mode(LedMode mode)
{
    switch (mode)
    {
        case LedMode::OFF:
            leds_clear();
            break;

        case LedMode::BOOTING:
            leds_set(0, 0, 0, 30);      // Blue
            break;

        case LedMode::OK:
            leds_set(0, 0, 30, 0);      // Green
            break;

        case LedMode::SENSOR_ERROR:
            leds_set(0, 30, 0, 0);      // Red
            break;
    }

    leds_show();
}