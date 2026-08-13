#include "buzzer.h"

Buzzer::Buzzer(uint pin) : _pin(pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

void Buzzer::on() {
    gpio_put(_pin, 1);
}

void Buzzer::off() {
    gpio_put(_pin, 0);
}

void Buzzer::alarm() {
    
}