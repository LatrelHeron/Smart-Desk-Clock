#pragma once

#include "pico/stdlib.h"

class Buzzer {
    public:
        Buzzer(uint pin);

        void on();
        void off();
        void alarm();

    private:
        uint _pin;
};
