#include "SDCardLowLevel.h"

#include <cstddef>
#include <cstdint>

namespace
{
    constexpr uint8_t CT_MMC   = 0x01;
    constexpr uint8_t CT_SD1   = 0x02;
    constexpr uint8_t CT_SD2   = 0x04;
    constexpr uint8_t CT_BLOCK = 0x08;

    constexpr uint8_t CMD0  = 0;
    constexpr uint8_t CMD1  = 1;
    constexpr uint8_t CMD8  = 8;
    constexpr uint8_t CMD9  = 9;
    constexpr uint8_t CMD16 = 16;
    constexpr uint8_t CMD17 = 17;
    constexpr uint8_t CMD24 = 24;
    constexpr uint8_t CMD55 = 55;
    constexpr uint8_t CMD58 = 58;

    constexpr uint8_t ACMD41 = 0x80 + 41;

    sd_low_level::Pins pins{};

    bool configured = false;
    bool initialized = false;
    bool slow_clock = true;

    uint8_t card_type = 0;
    uint64_t sector_count = 0;

    uint8_t transfer(uint8_t output)
    {
        uint8_t input = 0;

        // Software SPI mode 0.
        for (int bit = 7; bit >= 0; --bit)
        {
            gpio_put(
                pins.mosi,
                (output >> bit) & 1u
            );

            if (slow_clock)
            {
                sleep_us(1);
            }

            gpio_put(pins.clk, 1);

            input = static_cast<uint8_t>(
                (input << 1u) |
                (gpio_get(pins.miso) ? 1u : 0u)
            );

            if (slow_clock)
            {
                sleep_us(1);
            }

            gpio_put(pins.clk, 0);
        }

        return input;
    }

    void deselect()
    {
        gpio_put(pins.cs, 1);
        transfer(0xFF);
    }

    void select()
    {
        gpio_put(pins.cs, 0);
        transfer(0xFF);
    }

    bool waitReady(uint32_t timeout_ms)
    {
        const absolute_time_t timeout =
            make_timeout_time_ms(timeout_ms);

        do
        {
            if (transfer(0xFF) == 0xFF)
            {
                return true;
            }
        }
        while (!time_reached(timeout));

        return false;
    }

    bool waitForToken(
        uint8_t expected,
        uint32_t timeout_ms)
    {
        const absolute_time_t timeout =
            make_timeout_time_ms(timeout_ms);

        do
        {
            const uint8_t value = transfer(0xFF);

            if (value == expected)
            {
                return true;
            }

            if (value != 0xFF)
            {
                return false;
            }
        }
        while (!time_reached(timeout));

        return false;
    }

    uint8_t sendCommand(
        uint8_t command,
        uint32_t argument)
    {
        if ((command & 0x80u) != 0u)
        {
            command &= 0x7Fu;

            const uint8_t response =
                sendCommand(CMD55, 0);

            if (response > 1)
            {
                return response;
            }
        }

        deselect();
        select();

        if (!waitReady(500))
        {
            deselect();
            return 0xFF;
        }

        transfer(static_cast<uint8_t>(
            0x40u | command
        ));

        transfer(static_cast<uint8_t>(
            argument >> 24u
        ));

        transfer(static_cast<uint8_t>(
            argument >> 16u
        ));

        transfer(static_cast<uint8_t>(
            argument >> 8u
        ));

        transfer(static_cast<uint8_t>(
            argument
        ));

        uint8_t crc = 0x01;

        if (command == CMD0)
        {
            crc = 0x95;
        }
        else if (command == CMD8)
        {
            crc = 0x87;
        }

        transfer(crc);

        for (int attempt = 0;
             attempt < 10;
             ++attempt)
        {
            const uint8_t response =
                transfer(0xFF);

            if ((response & 0x80u) == 0)
            {
                return response;
            }
        }

        return 0xFF;
    }

    bool readSector(
        uint32_t sector,
        BYTE* buffer)
    {
        uint32_t address = sector;

        if ((card_type & CT_BLOCK) == 0)
        {
            address *= 512u;
        }

        if (sendCommand(CMD17, address) != 0)
        {
            deselect();
            return false;
        }

        if (!waitForToken(0xFE, 250))
        {
            deselect();
            return false;
        }

        for (size_t i = 0; i < 512; ++i)
        {
            buffer[i] = transfer(0xFF);
        }

        // Ignore CRC.
        transfer(0xFF);
        transfer(0xFF);

        deselect();
        return true;
    }

    bool writeSector(
        uint32_t sector,
        const BYTE* buffer)
    {
        uint32_t address = sector;

        if ((card_type & CT_BLOCK) == 0)
        {
            address *= 512u;
        }

        if (sendCommand(CMD24, address) != 0)
        {
            deselect();
            return false;
        }

        transfer(0xFF);
        transfer(0xFE);

        for (size_t i = 0; i < 512; ++i)
        {
            transfer(buffer[i]);
        }

        // Dummy CRC.
        transfer(0xFF);
        transfer(0xFF);

        const uint8_t response =
            transfer(0xFF) & 0x1F;

        if (response != 0x05)
        {
            deselect();
            return false;
        }

        const bool ready = waitReady(1000);

        deselect();
        return ready;
    }

    bool readCapacity()
    {
        uint8_t csd[16];

        if (sendCommand(CMD9, 0) != 0)
        {
            deselect();
            return false;
        }

        if (!waitForToken(0xFE, 250))
        {
            deselect();
            return false;
        }

        for (uint8_t& byte : csd)
        {
            byte = transfer(0xFF);
        }

        transfer(0xFF);
        transfer(0xFF);

        deselect();

        const uint8_t csd_structure =
            (csd[0] >> 6u) & 0x03u;

        if (csd_structure == 1)
        {
            const uint32_t c_size =
                (static_cast<uint32_t>(
                    csd[7] & 0x3F) << 16u) |
                (static_cast<uint32_t>(
                    csd[8]) << 8u) |
                csd[9];

            sector_count =
                (static_cast<uint64_t>(c_size) + 1u)
                * 1024u;

            return true;
        }

        if (csd_structure == 0)
        {
            const uint32_t read_block_length =
                csd[5] & 0x0F;

            const uint32_t c_size =
                (static_cast<uint32_t>(
                    csd[6] & 0x03) << 10u) |
                (static_cast<uint32_t>(
                    csd[7]) << 2u) |
                ((csd[8] & 0xC0) >> 6u);

            const uint32_t multiplier =
                (static_cast<uint32_t>(
                    csd[9] & 0x03) << 1u) |
                ((csd[10] & 0x80) >> 7u);

            const uint64_t block_count =
                (static_cast<uint64_t>(c_size) + 1u)
                << (multiplier + 2u);

            const uint64_t block_length =
                static_cast<uint64_t>(1u)
                << read_block_length;

            sector_count =
                block_count * block_length / 512u;

            return sector_count != 0;
        }

        return false;
    }
}

void sd_low_level::configure(
    const Pins& new_pins)
{
    pins = new_pins;

    gpio_init(pins.cs);
    gpio_set_dir(pins.cs, GPIO_OUT);
    gpio_put(pins.cs, 1);

    gpio_init(pins.clk);
    gpio_set_dir(pins.clk, GPIO_OUT);
    gpio_put(pins.clk, 0);

    gpio_init(pins.mosi);
    gpio_set_dir(pins.mosi, GPIO_OUT);
    gpio_put(pins.mosi, 1);

    gpio_init(pins.miso);
    gpio_set_dir(pins.miso, GPIO_IN);
    gpio_pull_up(pins.miso);

    configured = true;
}

DSTATUS sd_low_level::initialize()
{
    if (!configured)
    {
        return STA_NOINIT;
    }

    initialized = false;
    card_type = 0;
    sector_count = 0;
    slow_clock = true;

    gpio_put(pins.cs, 1);
    gpio_put(pins.clk, 0);

    // At least 74 clock cycles with CS high.
    for (int i = 0; i < 10; ++i)
    {
        transfer(0xFF);
    }

    uint8_t type = 0;

    if (sendCommand(CMD0, 0) == 1)
    {
        const absolute_time_t timeout =
            make_timeout_time_ms(2000);

        if (sendCommand(CMD8, 0x1AA) == 1)
        {
            uint8_t response[4];

            for (uint8_t& byte : response)
            {
                byte = transfer(0xFF);
            }

            if (response[2] == 0x01 &&
                response[3] == 0xAA)
            {
                while (!time_reached(timeout) &&
                       sendCommand(
                           ACMD41,
                           1UL << 30u) != 0)
                {
                    sleep_ms(10);
                }

                if (!time_reached(timeout) &&
                    sendCommand(CMD58, 0) == 0)
                {
                    for (uint8_t& byte : response)
                    {
                        byte = transfer(0xFF);
                    }

                    type = static_cast<uint8_t>(
                        CT_SD2 |
                        ((response[0] & 0x40)
                            ? CT_BLOCK
                            : 0)
                    );
                }
            }
        }
        else
        {
            uint8_t init_command;

            if (sendCommand(ACMD41, 0) <= 1)
            {
                type = CT_SD1;
                init_command = ACMD41;
            }
            else
            {
                type = CT_MMC;
                init_command = CMD1;
            }

            while (!time_reached(timeout) &&
                   sendCommand(init_command, 0) != 0)
            {
                sleep_ms(10);
            }

            if (time_reached(timeout) ||
                sendCommand(CMD16, 512) != 0)
            {
                type = 0;
            }
        }
    }

    card_type = type;
    deselect();

    if (type == 0)
    {
        return STA_NOINIT;
    }

    slow_clock = false;
    initialized = true;

    (void)readCapacity();

    return 0;
}

DSTATUS sd_low_level::status()
{
    return initialized ? 0 : STA_NOINIT;
}

DRESULT sd_low_level::read(
    BYTE* buffer,
    LBA_t sector,
    UINT count)
{
    if (!initialized)
    {
        return RES_NOTRDY;
    }

    if (buffer == nullptr || count == 0)
    {
        return RES_PARERR;
    }

    for (UINT i = 0; i < count; ++i)
    {
        if (!readSector(
                static_cast<uint32_t>(sector + i),
                buffer + i * 512u))
        {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

DRESULT sd_low_level::write(
    const BYTE* buffer,
    LBA_t sector,
    UINT count)
{
    if (!initialized)
    {
        return RES_NOTRDY;
    }

    if (buffer == nullptr || count == 0)
    {
        return RES_PARERR;
    }

    for (UINT i = 0; i < count; ++i)
    {
        if (!writeSector(
                static_cast<uint32_t>(sector + i),
                buffer + i * 512u))
        {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

DRESULT sd_low_level::ioctl(
    BYTE command,
    void* buffer)
{
    if (!initialized)
    {
        return RES_NOTRDY;
    }

    switch (command)
    {
        case CTRL_SYNC:
        {
            select();

            const bool ready =
                waitReady(1000);

            deselect();

            return ready
                ? RES_OK
                : RES_ERROR;
        }

        case GET_SECTOR_COUNT:
        {
            if (buffer == nullptr ||
                sector_count == 0)
            {
                return RES_ERROR;
            }

            *static_cast<LBA_t*>(buffer) =
                static_cast<LBA_t>(
                    sector_count
                );

            return RES_OK;
        }

        case GET_SECTOR_SIZE:
        {
            if (buffer == nullptr)
            {
                return RES_PARERR;
            }

            *static_cast<WORD*>(buffer) = 512;
            return RES_OK;
        }

        case GET_BLOCK_SIZE:
        {
            if (buffer == nullptr)
            {
                return RES_PARERR;
            }

            *static_cast<DWORD*>(buffer) = 1;
            return RES_OK;
        }

        default:
            return RES_PARERR;
    }
}