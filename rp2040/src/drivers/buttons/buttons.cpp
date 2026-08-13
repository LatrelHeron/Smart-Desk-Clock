#include "buttons.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

Button::Button(uint pin) : _pin(pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_down(pin);
}
