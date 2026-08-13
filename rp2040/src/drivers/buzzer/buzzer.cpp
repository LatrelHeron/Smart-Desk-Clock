#include "buzzer.h"

Buzzer::Buzzer(uint pin, volatile bool& btn2_flag) : _pin(pin), _btn2(btn2_flag) {
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
    for (int i = 0; i < 60; i++) {
        Buzzer::on();
        sleep_ms(495);
        Buzzer::off();
        sleep_ms(495);
        if (_btn2) {\
            _btn2 = false;
            break;
        }
    }
}