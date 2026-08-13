cmake_minimum_required(VERSION 3.13)


# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")



# Detect if the active kit is an ARM cross-compiler
if(CMAKE_CXX_COMPILER MATCHES "arm-none-eabi")
    message(STATUS "Detected that the current kit is a cross-compiler.")
    set(CrossCompiling 1)
else()
    message(STATUS "Detected that the current kit is a host compiler. Building the test harness.")
endif()

# Detect if the active kit is an ARM cross-compiler
if(CrossCompiling)
    # Yes, build for the RP2040

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)

    # Specify an increased oscillator startup delay to work around a potential hardware issue where the 
    # clock is slow to boot.
    add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

    # Pull in Raspberry Pi Pico SDK (must be before project)
    include(pico_sdk_import.cmake)

    project(cc3501-smart_desk_clock C CXX ASM)
    
    # Initialise the Raspberry Pi Pico SDK
    pico_sdk_init()

    add_executable(smart_desk_clock
        src/main.cpp
    )
    target_sources(smart_desk_clock 
        PUBLIC
        src/main.cpp
        src/drivers/logging/logging.cpp
        src/drivers/WS2812/leds.cpp
        src/drivers/LIS3DH/lis3dh.cpp
        src/drivers/microphone/microphone.cpp
        src/drivers/epaper/epaper_3in7.cpp
        src/drivers/SEN0546/temp_sensor.cpp
        src/drivers/rtc/rtc.cpp
        src/drivers/app/clock_app.cpp
        src/drivers/MicroSD/MicroSD.cpp
        src/drivers/MicroSD/fatfs/ff.c
        src/drivers/MicroSD/fatfs/ff.h
        src/drivers/MicroSD/fatfs/ffconf.h
        src/drivers/MicroSD/fatfs/diskio.h
        src/drivers/MicroSD/fatfs/diskio.cpp
        src/drivers/MicroSD/fatfs/SDCardLowLevel.h
        src/drivers/MicroSD/fatfs/SDCardLowLevel.cpp
        src/drivers/buttons/buttons.cpp
        src/drivers/buttons/buttons.h
        src/time_adjust.cpp
        src/time_adjust.h
        src/drivers/buzzer/buzzer.h
        src/drivers/buzzer/buzzer.cpp
        src/drivers/alarm/set_alarm.h
 
    )
    target_include_directories(smart_desk_clock
        PUBLIC 
        src/
    )

    pico_set_program_name(smart_desk_clock "cc3501-smart_desk_clock")
    pico_set_program_version(smart_desk_clock "0.2")

    # select UART for standard IO
    pico_enable_stdio_uart(smart_desk_clock 1)
    pico_enable_stdio_usb(smart_desk_clock 0)

    # compile the PIO file
    pico_generate_pio_header(smart_desk_clock ${CMAKE_CURRENT_LIST_DIR}/src/drivers/WS2812/WS2812.pio)

    # Add the standard library to the build
    target_link_libraries(smart_desk_clock
        pico_stdlib
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        hardware_pwm
        hardware_adc
    )

    pico_add_extra_outputs(smart_desk_clock)

else()

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 20)

    # We are building natively, so create the test harness instead
    project(cc3501-smart_desk_clock CXX)

    add_executable(smart_desk_clock)
    target_sources(smart_desk_clock 
        PUBLIC
        src/main.cpp
        src/drivers/logging/logging.cpp
        src/drivers/WS2812/leds.cpp
        src/drivers/LIS3DH/lis3dh.cpp
        src/drivers/microphone/microphone.cpp
        src/drivers/epaper/epaper_3in7.cpp
        src/drivers/SEN0546/temp_sensor.cpp
        src/drivers/rtc/rtc.cpp
        src/drivers/app/clock_app.cpp
        src/drivers/MicroSD/MicroSD.cpp
        tests/mocks/pico/stdlib.cpp
        tests/mocks/pico/time.cpp
        tests/mocks/hardware/gpio.cpp
        tests/mocks/hardware/pio.cpp
        tests/mocks/ws2812.cpp
        src/drivers/MicroSD/fatfs/ff.c
        src/drivers/MicroSD/fatfs/ff.h
        src/drivers/MicroSD/fatfs/ffconf.h
        src/drivers/MicroSD/fatfs/diskio.h
        src/drivers/MicroSD/fatfs/diskio.cpp
        src/drivers/MicroSD/fatfs/SDCardLowLevel.cpp
        src/drivers/MicroSD/fatfs/SDCardLowLevel.h
    )
    target_include_directories(smart_desk_clock
        PUBLIC 
        src/
        tests/
        tests/mocks/
    )
    target_compile_definitions(smart_desk_clock 
        PUBLIC
        TEST_HARNESS=1
    )

endif()

target_compile_definitions(smart_desk_clock 
    PUBLIC
    LOG_DRIVER_STYLE=${LogDriverImplementation}
)
