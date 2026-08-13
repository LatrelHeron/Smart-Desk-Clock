#include "time_adjust.h"
#include <stdio.h>

TimeAdjust::TimeAdjust(INS5699S& rtc, volatile bool& btn1_flag, volatile bool& btn2_flag, volatile bool& btn3_flag)
    : _rtc(rtc), _btn1(btn1_flag), _btn2(btn2_flag), _btn3(btn3_flag) {}

void TimeAdjust::advance_state() {
    switch (_state) {
        case State::EDIT_DAY: _state = State::EDIT_MONTH; printf("Editing: MONTH\n"); break;
        case State::EDIT_MONTH: _state = State::EDIT_YEAR; printf("Editing: DATE\n"); break;
        case State::EDIT_YEAR: _state = State::EDIT_HOUR; printf("Editing: HOUR\n"); break;
        case State::EDIT_HOUR:   _state = State::EDIT_MINUTE; printf("Editing: MINUTE\n"); break;
        case State::EDIT_MINUTE: _state = State::EDIT_SECOND; printf("Editing: SECOND\n"); break;
        case State::EDIT_SECOND: _state = State::DONE; break;
        case State::DONE: break;
    }
}

void TimeAdjust::increment_field() {
    switch (_state) {
        case State::EDIT_DAY:   _buffer.day   = (_buffer.day + 1) % 31; printf("Day: %02u\n", _buffer.day); break;
        case State::EDIT_MONTH:   _buffer.month   = ((_buffer.month + 1) % 12) + 1; printf("Month: %02u\n", _buffer.month); break;
        case State::EDIT_YEAR:   _buffer.year   = ((_buffer.year + 1) % 99) + 2000; printf("Year: %04u\n", _buffer.year); break;
        case State::EDIT_HOUR:   _buffer.hour   = (_buffer.hour + 1) % 24; printf("Hour: %02u\n", _buffer.hour); break;
        case State::EDIT_MINUTE: _buffer.minute = (_buffer.minute + 1) % 60; printf("Minute: %02u\n", _buffer.minute); break;
        case State::EDIT_SECOND: _buffer.second = (_buffer.second + 1) % 60; printf("Second: %02u\n", _buffer.second); break;
        case State::DONE: break;
    }
}

void TimeAdjust::run() {
    _buffer = _rtc.read_datetime(); // snapshot to edit
    _state = State::EDIT_DAY;
    printf("Entering time edit: DAY\n");

    while (_state != State::DONE) {
        if (_btn1) { _btn1 = false; advance_state(); }
        if (_btn2) { _btn2 = false; increment_field(); }
        sleep_ms(10);
    }

    if (_rtc.set_datetime(_buffer)) {
        printf("RTC updated: %02u/%02u/%04u %02u:%02u:%02u\n", _buffer.day, _buffer.month, _buffer.year, _buffer.hour, _buffer.minute, _buffer.second);
    } else {
        printf("ERROR: failed to write RTC\n");
    }
}