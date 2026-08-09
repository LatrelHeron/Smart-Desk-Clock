#include "drivers/epaper/epaper_3in7.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdio.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/spi.h"

namespace epaper
{

    namespace
    {

        // Compact 5x7 uppercase/digit font. Each byte is one vertical column.
        struct Glyph
        {
            char c;
            uint8_t col[5];
        };

        constexpr Glyph FONT[] = {
            {' ', {0x00, 0x00, 0x00, 0x00, 0x00}},
            {'-', {0x08, 0x08, 0x08, 0x08, 0x08}},
            {'.', {0x00, 0x60, 0x60, 0x00, 0x00}},
            {'0', {0x3E, 0x51, 0x49, 0x45, 0x3E}},
            {'1', {0x00, 0x42, 0x7F, 0x40, 0x00}},
            {'2', {0x42, 0x61, 0x51, 0x49, 0x46}},
            {'3', {0x21, 0x41, 0x45, 0x4B, 0x31}},
            {'4', {0x18, 0x14, 0x12, 0x7F, 0x10}},
            {'5', {0x27, 0x45, 0x45, 0x45, 0x39}},
            {'6', {0x3C, 0x4A, 0x49, 0x49, 0x30}},
            {'7', {0x01, 0x71, 0x09, 0x05, 0x03}},
            {'8', {0x36, 0x49, 0x49, 0x49, 0x36}},
            {'9', {0x06, 0x49, 0x49, 0x29, 0x1E}},
            {'A', {0x7E, 0x11, 0x11, 0x11, 0x7E}},
            {'B', {0x7F, 0x49, 0x49, 0x49, 0x36}},
            {'C', {0x3E, 0x41, 0x41, 0x41, 0x22}},
            {'D', {0x7F, 0x41, 0x41, 0x22, 0x1C}},
            {'E', {0x7F, 0x49, 0x49, 0x49, 0x41}},
            {'F', {0x7F, 0x09, 0x09, 0x09, 0x01}},
            {'G', {0x3E, 0x41, 0x49, 0x49, 0x7A}},
            {'H', {0x7F, 0x08, 0x08, 0x08, 0x7F}},
            {'I', {0x00, 0x41, 0x7F, 0x41, 0x00}},
            {'J', {0x20, 0x40, 0x41, 0x3F, 0x01}},
            {'K', {0x7F, 0x08, 0x14, 0x22, 0x41}},
            {'L', {0x7F, 0x40, 0x40, 0x40, 0x40}},
            {'M', {0x7F, 0x02, 0x0C, 0x02, 0x7F}},
            {'N', {0x7F, 0x04, 0x08, 0x10, 0x7F}},
            {'O', {0x3E, 0x41, 0x41, 0x41, 0x3E}},
            {'P', {0x7F, 0x09, 0x09, 0x09, 0x06}},
            {'Q', {0x3E, 0x41, 0x51, 0x21, 0x5E}},
            {'R', {0x7F, 0x09, 0x19, 0x29, 0x46}},
            {'S', {0x46, 0x49, 0x49, 0x49, 0x31}},
            {'T', {0x01, 0x01, 0x7F, 0x01, 0x01}},
            {'U', {0x3F, 0x40, 0x40, 0x40, 0x3F}},
            {'V', {0x1F, 0x20, 0x40, 0x20, 0x1F}},
            {'W', {0x3F, 0x40, 0x38, 0x40, 0x3F}},
            {'X', {0x63, 0x14, 0x08, 0x14, 0x63}},
            {'Y', {0x07, 0x08, 0x70, 0x08, 0x07}},
            {'Z', {0x61, 0x51, 0x49, 0x45, 0x43}},
        };

        const uint8_t *glyph_for(char c)
        {
            if (c >= 'a' && c <= 'z')
                c = static_cast<char>(c - 'a' + 'A');
            for (const auto &glyph : FONT)
            {
                if (glyph.c == c)
                    return glyph.col;
            }
            return FONT[0].col;
        }

    } // namespace

    Epaper3in7::Epaper3in7(spi_inst_t *spi, Pins pins)
        : spi_(spi), pins_(pins) {}

    bool Epaper3in7::init()
    {
        spi_init(spi_, 4'000'000);
        spi_set_format(spi_, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

        gpio_set_function(pins_.sck, GPIO_FUNC_SPI);
        gpio_set_function(pins_.mosi, GPIO_FUNC_SPI);

        gpio_init(pins_.cs);
        gpio_set_dir(pins_.cs, GPIO_OUT);
        gpio_put(pins_.cs, 1);

        gpio_init(pins_.dc);
        gpio_set_dir(pins_.dc, GPIO_OUT);
        gpio_put(pins_.dc, 0);

        gpio_init(pins_.reset);
        gpio_set_dir(pins_.reset, GPIO_OUT);
        gpio_put(pins_.reset, 1);

        gpio_init(pins_.busy);
        gpio_set_dir(pins_.busy, GPIO_IN);
        gpio_pull_up(pins_.busy);

        hardware_reset();

        // UC8253/UC8171-style 3.7-inch panel startup used by WeAct's reference driver.
        command(0x04); // POWER_ON
        if (!wait_while_busy())
            return false;

        command(0x00); // PANEL_SETTING
        data(0x1F);
        data(0x0D);

        command(0x50); // VCOM_AND_DATA_INTERVAL
        data(0x97);

        return true;
    }

    void Epaper3in7::hardware_reset()
    {
        gpio_put(pins_.reset, 1);
        sleep_ms(20);
        gpio_put(pins_.reset, 0);
        sleep_ms(10);
        gpio_put(pins_.reset, 1);
        sleep_ms(20);
    }

    void Epaper3in7::command(uint8_t value)
    {
        gpio_put(pins_.dc, 0);
        gpio_put(pins_.cs, 0);
        spi_write_blocking(spi_, &value, 1);
        gpio_put(pins_.cs, 1);
    }

    void Epaper3in7::data(uint8_t value)
    {
        gpio_put(pins_.dc, 1);
        gpio_put(pins_.cs, 0);
        spi_write_blocking(spi_, &value, 1);
        gpio_put(pins_.cs, 1);
    }

    void Epaper3in7::data_buffer(const uint8_t *values, std::size_t length)
    {
        gpio_put(pins_.dc, 1);
        gpio_put(pins_.cs, 0);
        spi_write_blocking(spi_, values, length);
        gpio_put(pins_.cs, 1);
    }

    bool Epaper3in7::wait_while_busy(uint32_t timeout_ms)
    {
        const absolute_time_t deadline = make_timeout_time_ms(timeout_ms);

        // On this WeAct 3.7-inch UC8253 board BUSY is active LOW.
        while (gpio_get(pins_.busy) == 0)
        {
            if (absolute_time_diff_us(get_absolute_time(), deadline) <= 0)
            {
                return false;
            }
            sleep_ms(10);
        }
        sleep_ms(100);
        return true;
    }
    bool Epaper3in7::display(const uint8_t *image)
    {
        if (image == nullptr)
            return false;

        // The controller expects previous image RAM followed by new image RAM.
        static uint8_t previous[BUFFER_SIZE];
        static bool previous_valid = false;

        if (!previous_valid)
        {
            std::memset(previous, 0xFF, BUFFER_SIZE);
            previous_valid = true;
        }

        command(0x10); // DATA_START_TRANSMISSION_1
        data_buffer(previous, BUFFER_SIZE);

        command(0x13); // DATA_START_TRANSMISSION_2
        data_buffer(image, BUFFER_SIZE);

        command(0x12); // DISPLAY_REFRESH
        sleep_ms(1);
        if (!wait_while_busy())
            return false;

        std::memcpy(previous, image, BUFFER_SIZE);
        return true;
    }

    static Rotation current_rotation = Rotation::Portrait;

    void set_rotation(Rotation rotation)
    {
        current_rotation = rotation;
    }

    void Epaper3in7::send_partial_region(
        uint8_t command_value,
        const uint8_t *image,
        int x,
        int y,
        int width,
        int height)
    {
        command(command_value);

        /*
         * UC8253 partial-area header:
         *
         * byte 1: X bits 9:8
         * byte 2: X bits 7:3, with lower three bits zero
         * byte 3: Y bits 9:8
         * byte 4: Y bits 7:0
         * byte 5: width bits 9:8
         * byte 6: width bits 7:3
         * byte 7: height bits 9:8
         * byte 8: height bits 7:0
         */
        data(static_cast<uint8_t>((x >> 8) & 0x03));
        data(static_cast<uint8_t>(x & 0xF8));

        data(static_cast<uint8_t>((y >> 8) & 0x03));
        data(static_cast<uint8_t>(y & 0xFF));

        data(static_cast<uint8_t>((width >> 8) & 0x03));
        data(static_cast<uint8_t>(width & 0xF8));

        data(static_cast<uint8_t>((height >> 8) & 0x03));
        data(static_cast<uint8_t>(height & 0xFF));

        const int bytes_per_row = width / 8;
        const int framebuffer_bytes_per_row = WIDTH / 8;

        for (int row = 0; row < height; ++row)
        {
            const std::size_t source_index =
                static_cast<std::size_t>(y + row) *
                    framebuffer_bytes_per_row +
                static_cast<std::size_t>(x / 8);

            data_buffer(
                &image[source_index],
                static_cast<std::size_t>(bytes_per_row));
        }
    }

    void Epaper3in7::send_partial_refresh_command(
        int x,
        int y,
        int width,
        int height)
    {
        command(0x16); // PARTIAL_DISPLAY_REFRESH

        /*
         * DFV_EN = 0.
         *
         * The two lower bits contain X bits 9:8.
         */
        data(static_cast<uint8_t>((x >> 8) & 0x03));
        data(static_cast<uint8_t>(x & 0xF8));

        data(static_cast<uint8_t>((y >> 8) & 0x03));
        data(static_cast<uint8_t>(y & 0xFF));

        data(static_cast<uint8_t>((width >> 8) & 0x03));
        data(static_cast<uint8_t>(width & 0xF8));

        data(static_cast<uint8_t>((height >> 8) & 0x03));
        data(static_cast<uint8_t>(height & 0xFF));
    }
    bool Epaper3in7::display_partial(
        const uint8_t *image,
        int x,
        int y,
        int width,
        int height)
    {
        if (image == nullptr)
        {
            return false;
        }

        if (!previous_valid_)
        {
            // A full frame is required before partial updates.
            return display(image);
        }

        if (x < 0 ||
            y < 0 ||
            width <= 0 ||
            height <= 0 ||
            x + width > WIDTH ||
            y + height > HEIGHT)
        {
            return false;
        }

        /*
         * The controller requires X and width to be byte-aligned.
         */
        if ((x % 8) != 0 ||
            (width % 8) != 0)
        {
            return false;
        }

        // Send the old pixels currently visible on the panel.
        send_partial_region(
            0x14,
            previous_,
            x,
            y,
            width,
            height);

        // Send the replacement pixels.
        send_partial_region(
            0x15,
            image,
            x,
            y,
            width,
            height);

        send_partial_refresh_command(
            x,
            y,
            width,
            height);

        sleep_ms(1);

        if (!wait_while_busy())
        {
            return false;
        }

        const int bytes_per_row = width / 8;
        const int framebuffer_bytes_per_row = WIDTH / 8;

        for (int row = 0; row < height; ++row)
        {
            const std::size_t index =
                static_cast<std::size_t>(y + row) *
                    framebuffer_bytes_per_row +
                static_cast<std::size_t>(x / 8);

            std::memcpy(
                &previous_[index],
                &image[index],
                static_cast<std::size_t>(bytes_per_row));
        }

        return true;
    }

    bool Epaper3in7::clear(bool black)
    {
        static uint8_t buffer[BUFFER_SIZE];
        std::memset(buffer, black ? 0x00 : 0xFF, sizeof(buffer));
        return display(buffer);
    }

    void Epaper3in7::sleep()
    {
        command(0x02); // POWER_OFF
        wait_while_busy();
        command(0x07); // DEEP_SLEEP
        data(0xA5);
    }

    void buffer_clear(uint8_t *buffer, bool black)
    {
        if (buffer == nullptr)
            return;
        std::memset(buffer, black ? 0x00 : 0xFF, BUFFER_SIZE);
    }

    void set_pixel(uint8_t *buffer, int x, int y, bool black)
    {
        if (buffer == nullptr || x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return;

        const std::size_t index = static_cast<std::size_t>(y) * (WIDTH / 8) + (x / 8);
        const uint8_t mask = static_cast<uint8_t>(0x80u >> (x & 7));

        if (black)
        {
            buffer[index] &= static_cast<uint8_t>(~mask);
        }
        else
        {
            buffer[index] |= mask;
        }
    }

    void draw_rect(uint8_t *buffer, int x, int y, int width, int height, bool black, bool filled)
    {
        if (width <= 0 || height <= 0)
            return;

        if (filled)
        {
            for (int yy = y; yy < y + height; ++yy)
            {
                for (int xx = x; xx < x + width; ++xx)
                {
                    set_pixel(buffer, xx, yy, black);
                }
            }
            return;
        }

        for (int xx = x; xx < x + width; ++xx)
        {
            set_pixel(buffer, xx, y, black);
            set_pixel(buffer, xx, y + height - 1, black);
        }
        for (int yy = y; yy < y + height; ++yy)
        {
            set_pixel(buffer, x, yy, black);
            set_pixel(buffer, x + width - 1, yy, black);
        }
    }

    void draw_text(uint8_t *buffer, int x, int y, const char *text, int scale)
    {
        if (buffer == nullptr || text == nullptr || scale < 1)
            return;

        int cursor_x = x;
        for (const char *p = text; *p != '\0'; ++p)
        {
            const uint8_t *columns = glyph_for(*p);

            for (int col = 0; col < 5; ++col)
            {
                for (int row = 0; row < 7; ++row)
                {
                    if ((columns[col] >> row) & 0x01u)
                    {
                        for (int sy = 0; sy < scale; ++sy)
                        {
                            for (int sx = 0; sx < scale; ++sx)
                            {
                                set_pixel(buffer,
                                          cursor_x + col * scale + sx,
                                          y + row * scale + sy,
                                          true);
                            }
                        }
                    }
                }
            }
            cursor_x += 6 * scale;
        }
    }

} // namespace epaper
