#pragma once

#include <stdint.h>

#include "pico/stdlib.h"

extern "C" {
#include "diskio.h"
}

namespace sd_low_level
{
    struct Pins
    {
        uint cs;
        uint clk;
        uint mosi;
        uint miso;
    };

    void configure(const Pins& pins);

    DSTATUS initialize();
    DSTATUS status();

    DRESULT read(
        BYTE* buffer,
        LBA_t sector,
        UINT count
    );

    DRESULT write(
        const BYTE* buffer,
        LBA_t sector,
        UINT count
    );

    DRESULT ioctl(
        BYTE command,
        void* buffer
    );
}