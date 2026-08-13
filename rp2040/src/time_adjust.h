#include "pico/stdlib.h"
#include "drivers/rtc/rtc.h"

class TimeAdjust {
public:
    // References to the ISR-set flags declared in main.cpp
    TimeAdjust(INS5699S& rtc, volatile bool& btn1_flag, volatile bool& btn2_flag, volatile bool& btn3_flag);

    // Blocks until the user has stepped through hour/minute/second and saved.
    void run();

private:
    enum class State { EDIT_DAY, EDIT_MONTH, EDIT_YEAR, EDIT_HOUR, EDIT_MINUTE, EDIT_SECOND, DONE };

    INS5699S& _rtc;
    volatile bool& _btn1;
    volatile bool& _btn2;
    volatile bool& _btn3;
    DateTime _buffer{};
    State _state = State::EDIT_YEAR;

    void advance_state();
    void increment_field();
};
