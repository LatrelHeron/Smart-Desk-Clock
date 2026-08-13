#include "pico/stdlib.h"
#include "drivers/rtc/rtc.h"

class SetAlarm {
public:
    // References to the ISR-set flags declared in main.cpp
    SetAlarm(INS5699S& rtc, volatile bool& btn1_flag, volatile bool& btn2_flag, volatile bool& btn3_flag);

    void set_alarm();
    bool check_alarm();

    bool alarm_flag;
private:
    enum class State { EDIT_HOUR, EDIT_MINUTE, DONE };

    INS5699S& _rtc;
    volatile bool& _btn1;
    volatile bool& _btn2;
    volatile bool& _btn3;
    DateTime _buffer{};
    DateTime _alarm_time{};
    State _state = State::EDIT_HOUR;

    void advance_state();
    void increment_field();
    void decrement_field();
};
