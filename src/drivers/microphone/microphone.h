#pragma once

#include <stdint.h>

void mic_init();

void mic_read(uint16_t* buffer, 
uint32_t sample_count);