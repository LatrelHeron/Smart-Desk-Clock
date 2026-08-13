#pragma once

#include "pico/stdlib.h"

class Buzzer {
    public:
        Buzzer(uint pin);

        void on();
        void off();

    private:
        uint _pin;
};
