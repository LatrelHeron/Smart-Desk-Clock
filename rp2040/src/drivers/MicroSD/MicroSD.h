#pragma once

#include <string>
#include "hardware/spi.h"
#include "ff.h"


class MicroSD {
    public:
        MicroSD(
            spi_inst_t* spi,
            uint dat3,
            uint cmd,
            uint clk,
            uint dat0,
            uint cd,
            uint baudrate
        );

        bool init();

        bool mount();

        bool isMounted() const;

        bool WriteFile(const std::string& filename,
                        const std::string& text);

        bool appendFile(const std::string& filename,
                        const std::string& text);

        bool readFile(const std::string& filename,
                  std::string& output);

        bool exists(const std::string& filename);

        bool remove(const std::string& filename);
    
    private:
        spi_inst_t* _spi;

        uint _mosi;
        uint _miso;
        uint _sck;
        uint _cs;

        uint _baudrate;

        FATFS _fatfs;

        bool _mounted;
};
