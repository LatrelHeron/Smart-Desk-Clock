#pragma once

#include <cstddef>
#include <cstdint>

#include "pico/stdlib.h"
#include "hardware/spi.h"

namespace epaper {

constexpr int WIDTH = 240;
constexpr int HEIGHT = 416;
constexpr std::size_t BUFFER_SIZE = WIDTH * HEIGHT / 8;

struct Pins {
    uint sck;
    uint mosi;
    uint cs;
    uint dc;
    uint reset;
    uint busy;
};

class Epaper3in7 {
public:
    Epaper3in7(spi_inst_t* spi, Pins pins);

    bool init();
    bool display(const std::uint8_t* image);
    bool clear(bool black = false);
    void sleep();

private:
    spi_inst_t* spi_;
    Pins pins_;

    void hardware_reset();
    void command(std::uint8_t value);
    void data(std::uint8_t value);

    void data_buffer(
        const std::uint8_t* values,
        std::size_t length
    );

    bool wait_while_busy(
        std::uint32_t timeout_ms = 15000
    );
};

void buffer_clear(
    std::uint8_t* buffer,
    bool black = false
);

void set_pixel(
    std::uint8_t* buffer,
    int x,
    int y,
    bool black
);

void draw_rect(
    std::uint8_t* buffer,
    int x,
    int y,
    int width,
    int height,
    bool black,
    bool filled
);

void draw_text(
    std::uint8_t* buffer,
    int x,
    int y,
    const char* text,
    int scale = 2
);

} // namespace epaper