#pragma once

#include "pico/stdlib.h"

class Button {
    public:
        explicit Button(
            uint pin
        );

        bool was_pressed();

    private:
        uint _pin;
        volatile bool _pressed_flag = false;

        void handle_interrupt();

        static void gpio_callback(uint gpio, uint32_t events);
        static Button* instances[30];
};
