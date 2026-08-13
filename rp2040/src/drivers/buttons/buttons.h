#pragma once

#include "pico/stdlib.h"

class button
{
    public:
        button(
            uint pin
        );

        bool init();

    private:
        uint _pin;

}
