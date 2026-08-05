#pragma once

#include <cstddef>
#include <cstdint>

#include "pico/stdlib.h"
#include "hardware/spi.h"

namespace epaper
{

    constexpr int WIDTH = 240;
    constexpr int HEIGHT = 416;
    constexpr std::size_t BUFFER_SIZE = WIDTH * HEIGHT / 8;

    struct Pins
    {
        uint sck;
        uint mosi;
        uint cs;
        uint dc;
        uint reset;
        uint busy;
    };

    class Epaper3in7
    {
    public:
        Epaper3in7(spi_inst_t *spi, Pins pins);

        bool init();
        bool Epaper3in7::display(const uint8_t *image)
        {
            if (image == nullptr)
            {
                return false;
            }

            if (!previous_valid_)
            {
                std::memset(
                    previous_,
                    0xFF,
                    BUFFER_SIZE);

                previous_valid_ = true;
            }

            command(0x10); // DATA_START_TRANSMISSION_1
            data_buffer(previous_, BUFFER_SIZE);

            command(0x13); // DATA_START_TRANSMISSION_2
            data_buffer(image, BUFFER_SIZE);

            command(0x12); // DISPLAY_REFRESH
            sleep_ms(1);

            if (!wait_while_busy())
            {
                return false;
            }

            std::memcpy(
                previous_,
                image,
                BUFFER_SIZE);

            return true;
        }
        bool display_partial(
            const std::uint8_t *image,
            int x,
            int y,
            int width,
            int height);
        bool clear(bool black = false);
        void sleep();

    private:
        spi_inst_t *spi_;
        Pins pins_;

        void hardware_reset();
        void command(std::uint8_t value);
        void data(std::uint8_t value);

        void data_buffer(
            const std::uint8_t *values,
            std::size_t length);

        bool wait_while_busy(
            std::uint32_t timeout_ms = 15000);

        std::uint8_t previous_[BUFFER_SIZE]{};
        bool previous_valid_ = false;

        void send_partial_region(
            std::uint8_t command_value,
            const std::uint8_t *image,
            int x,
            int y,
            int width,
            int height);

        void send_partial_refresh_command(
            int x,
            int y,
            int width,
            int height);
        std::uint8_t previous_[BUFFER_SIZE]{};
        bool previous_valid_ = false;
    };

    void buffer_clear(
        std::uint8_t *buffer,
        bool black = false);

    void set_pixel(
        std::uint8_t *buffer,
        int x,
        int y,
        bool black);

    void draw_rect(
        std::uint8_t *buffer,
        int x,
        int y,
        int width,
        int height,
        bool black,
        bool filled);

    void draw_text(
        std::uint8_t *buffer,
        int x,
        int y,
        const char *text,
        int scale = 2);

    bool display_partial(
        const std::uint8_t *image,
        int x,
        int y,
        int width,
        int height);

} // namespace epaper