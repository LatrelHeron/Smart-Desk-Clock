#include "set_alarm.h"
#include <stdio.h>

SetAlarm::SetAlarm(INS5699S& rtc, volatile bool& btn1_flag, volatile bool& btn2_flag, volatile bool& btn3_flag)
    : _rtc(rtc), _btn1(btn1_flag), _btn2(btn2_flag), _btn3(btn3_flag) {}

void SetAlarm::advance_state() {
    switch (_state) {
        case State::EDIT_HOUR:   _state = State::EDIT_MINUTE; printf("Alarm: MINUTE\n"); break;
        case State::EDIT_MINUTE: _state = State::DONE; break;
        case State::DONE: break;
    }
}

void SetAlarm::increment_field() {
    switch (_state) {
        case State::EDIT_HOUR:   _buffer.hour   = (_buffer.hour + 1) % 24; printf("Hour: %02u\n", _buffer.hour); break;
        case State::EDIT_MINUTE: _buffer.minute = (_buffer.minute + 1) % 60; printf("Minute: %02u\n", _buffer.minute); break;
        case State::DONE: break;
    }
}

void SetAlarm::decrement_field() {
    switch (_state) {
        case State::EDIT_HOUR:   _buffer.hour   = (_buffer.hour == 0) ? 23 : _buffer.hour - 1; printf("Hour: %02u\n", _buffer.hour); break;
        case State::EDIT_MINUTE: _buffer.minute = (_buffer.minute == 0) ? 60 : _buffer.minute - 1; printf("Minute: %02u\n", _buffer.minute); break;
        case State::DONE: break;
    }
}

const DateTime SetAlarm::set_alarm() {
    _buffer = _rtc.read_datetime(); // snapshot to edit
    _state = State::EDIT_HOUR;
    printf("Entering alarm hour:\n");

    while (_state != State::DONE) {
        if (_btn1) { _btn1 = false; advance_state(); }
        if (_btn2) { _btn2 = false; increment_field(); }
        if (_btn3) { _btn3 = false; decrement_field(); }
        sleep_ms(10);
    }

    printf("Alarm set: %02u:%02u\n", _buffer.hour, _buffer.minute);
    _alarm_time.hour = _buffer.hour;
    _alarm_time.minute = _buffer.minute;
}

bool SetAlarm::check_alarm() {
    _buffer = _rtc.read_datetime();
    return (_buffer.hour == _alarm_time.hour && _buffer.minute == _alarm_time.minute);
}