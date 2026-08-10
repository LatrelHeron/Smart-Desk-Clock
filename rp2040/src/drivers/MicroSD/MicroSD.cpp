#include "MicroSD.h"
#include "fatfs/SDCardLowLevel.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>

MicroSD::MicroSD(
    uint dat3, 
    uint clk, 
    uint cmd, 
    uint dat0)
    : _dat3(dat3),
    _clk(clk),
    _cmd(cmd),
    _dat0(dat0) {
    
}

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

     sd_low_level::configure({
        .cs = _dat3,
        .clk = _clk,
        .mosi = _cmd,
        .miso = _dat0
    });

    last_result_ = f_mount(
        &filesystem_,
        "",
        1
    );

    return last_result_ == FR_OK;
}

bool MicroSD::writeData(
    const char* filename,
    std::string temperature,
    std::string humidity,
    std::string year,
    std::string month,
    std::string day,
    std::string hour,
    std::string minute
)
{
    const char* text = (day + "/" + month + "/" + year + " " + hour + ":" + minute + "," + temperature + "," + humidity + "\r\n").c_str();
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

std::vector<std::string> MicroSD::get_next_event() {
    // Get event from events.txt file on SD card
    std::string event;
    MicroSD::readText("events.txt", event);

    // Manipulate event string to present on display
    std::vector<std::string> lines;
    std::stringstream ss(event);
    std::string line;
    while (std::getline(ss, line, ',')) {
        lines.push_back(line);
    }
    return lines;
}

void deleteLine() {
    
}

