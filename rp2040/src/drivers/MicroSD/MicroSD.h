#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "pico/stdlib.h"

extern "C" {
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
}

class MicroSD
{
public:
    MicroSD(
        uint dat3,
        uint clk,
        uint cmd,
        uint dat0
    );

    bool init();

    bool writeData(
        const char* filename,
        std::string temperature,
        std::string humidity,
        std::string year,
        std::string month,
        std::string day,
        std::string hour,
        std::string minute
    );

    bool readText(
        const char* filename,
        std::string& output
    );

    std::vector<std::string> get_next_event();

    void deleteLine(
        const char* filename
    );

    FRESULT lastResult() const;

    // Called by diskio.c
    DSTATUS diskInitialize();

    DRESULT diskRead(
        BYTE* buffer,
        LBA_t sector,
        UINT count
    );

    DRESULT diskWrite(
        const BYTE* buffer,
        LBA_t sector,
        UINT count
    );

    DRESULT diskIoctl(
        BYTE command,
        void* buffer
    );

    static MicroSD* active();

private:
    uint _dat3;
    uint _clk;
    uint _cmd;
    uint _dat0;

    FATFS filesystem_{};

    FRESULT last_result_ = FR_NOT_ENABLED;

    bool initialized_ = false;
    bool mounted_ = false;
    bool high_capacity_ = false;
    bool slow_clock_ = true;

    static MicroSD* active_instance_;

    uint8_t transfer(uint8_t value);

    void select();
    void deselect();

    bool waitReady(uint32_t timeout_ms);

    uint8_t sendCommand(
        uint8_t command,
        uint32_t argument
    );

    bool readSector(
        uint32_t sector,
        BYTE* buffer
    );

    bool writeSector(
        uint32_t sector,
        const BYTE* buffer
    );
};