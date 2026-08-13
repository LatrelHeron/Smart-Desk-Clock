#include "time_adjust.h"
#include <stdio.h>

TimeAdjust::TimeAdjust(INS5699S& rtc, volatile bool& btn1_flag, volatile bool& btn2_flag)
    : _rtc(rtc), _btn1(btn1_flag), _btn2(btn2_flag) {}

void TimeAdjust::advance_state() {
    switch (_state) {
        case State::EDIT_HOUR:   _state = State::EDIT_MINUTE; printf("Editing: MINUTE\n"); break;
        case State::EDIT_MINUTE: _state = State::EDIT_SECOND; printf("Editing: SECOND\n"); break;
        case State::EDIT_SECOND: _state = State::DONE; break;
        case State::DONE: break;
    }
}

void TimeAdjust::increment_field() {
    switch (_state) {
        case State::EDIT_HOUR:   _buffer.hour   = (_buffer.hour + 1) % 24; printf("Hour: %02u\n", _buffer.hour); break;
        case State::EDIT_MINUTE: _buffer.minute = (_buffer.minute + 1) % 60; printf("Minute: %02u\n", _buffer.minute); break;
        case State::EDIT_SECOND: _buffer.second = (_buffer.second + 1) % 60; printf("Second: %02u\n", _buffer.second); break;
        case State::DONE: break;
    }
}

void TimeAdjust::run() {
    _buffer = _rtc.read_datetime(); // snapshot to edit
    _state = State::EDIT_HOUR;
    printf("Entering time edit: HOUR\n");

    while (_state != State::DONE) {
        if (_btn1) { _btn1 = false; advance_state(); }
        if (_btn2) { _btn2 = false; increment_field(); }
        sleep_ms(10); // avoid busy-spinning the CPU at 100%
    }

    if (_rtc.set_datetime(_buffer)) {
        printf("RTC updated: %02u:%02u:%02u\n", _buffer.hour, _buffer.minute, _buffer.second);
    } else {
        printf("ERROR: failed to write RTC\n");
    }
}