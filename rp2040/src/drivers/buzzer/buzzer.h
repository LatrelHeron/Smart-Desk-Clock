#pragma once

#include "pico/stdlib.h"

class Buzzer {
    public:
        Buzzer(uint pin, volatile bool& btn2_flag);

        void on();
        void off();
        void alarm();

    private:
        uint _pin;
        volatile bool& _btn2;
};
