#include "MicroSD.h"

#include <algorithm>
#include <cstring>

bool MicroSD::init() {
    gpio_init(_dat3);
    gpio_set_dir(_dat3, GPIO_OUT);
    gpio_put(_dat3, 1);

    gpio_init(_clk);
    gpio_set_dir(_clk, GPIO_OUT);
    gpio_put(_clk, 0);

    gpio_init(_cmd);
    gpio_set_dir(_cmd, GPIO_OUT);
    gpio_put(_cmd, 1);

    gpio_init(_dat0);
    gpio_set_dir(_dat0, GPIO_IN);
    gpio_pull_up(_dat0);
}


bool MicroSD::appendText(const char* filename, const char* text)
{
    FIL file;
    UINT written;

    FRESULT result = f_open(
        &file,
        filename,
        FA_OPEN_APPEND | FA_WRITE
    );

    if (result != FR_OK)
        return false;

    result = f_write(
        &file,
        text,
        strlen(text),
        &written
    );

    f_close(&file);

    return (result == FR_OK && written == strlen(text));
}

bool MicroSD::readText(
    const char* filename,
    std::string& output)
{
    FIL file;

    last_result_ = f_open(
        &file,
        filename,
        FA_READ
    );

    if (last_result_ != FR_OK) {
        return false;
    }

    output.clear();

    char buffer[128];

    while (true) {
        UINT bytes_read = 0;

        last_result_ = f_read(
            &file,
            buffer,
            sizeof(buffer),
            &bytes_read
        );

        if (last_result_ != FR_OK) {
            f_close(&file);
            return false;
        }

        output.append(buffer, bytes_read);

        if (bytes_read < sizeof(buffer)) {
            break;
        }
    }

    return f_close(&file) == FR_OK;
}

