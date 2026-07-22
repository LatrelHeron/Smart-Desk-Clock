#include "lis3dh.h"

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define ACCEL_I2C i2c0 
#define ACCEL_SDA_PIN 16
#define ACCEL_SCL_PIN 17

#define LIS3DH_ADDR 0x19

#define WHO_AM_I 0x0F
#define CTRL_REG1 0x20
#define CTRL_REG4 0x23
#define OUT_X_L 0x28

static void write_reg(uint8_t reg, uint8_t value){
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(ACCEL_I2C, LIS3DH_ADDR, data, 2, false);
}

static uint8_t read_reg(uint8_t reg)
{
    uint8_t value = 0;
    i2c_write_blocking(ACCEL_I2C, LIS3DH_ADDR, &reg, 1, true);
    i2c_read_blocking(ACCEL_I2C, LIS3DH_ADDR, &value, 1, false);
    return value;
}

bool lis3dh_init()
{
    i2c_init(ACCEL_I2C, 400 * 1000);

    gpio_set_function(ACCEL_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(ACCEL_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(ACCEL_SDA_PIN);
    gpio_pull_up(ACCEL_SCL_PIN);

    sleep_ms(10);

    uint8_t id = read_reg(WHO_AM_I);

    if (id != 0x33) {
        printf("LIS3DH not found. WHO_AM_I = 0x%02X\r\n", id);
        return false;
    }
    printf("LIS3DH found. WHO_AM_I = 0x%02X\r\n", id);

    write_reg(CTRL_REG1, 0x57); // 100 Hz, normal mode, X/Y/Z enabled
    write_reg(CTRL_REG4, 0x80); // BDU enabled, ±2g, normal mode

    return true;
}

lis3dh_raw_t lis3dh_read_raw()
{
    uint8_t reg = OUT_X_L | 0x80;
    uint8_t data[6];

    i2c_write_blocking(ACCEL_I2C, LIS3DH_ADDR, &reg, 1, true);
    i2c_read_blocking(ACCEL_I2C, LIS3DH_ADDR, data, 6, false);

    int16_t x = (int16_t)((data[1] << 8) | data[0]);
    int16_t y = (int16_t)((data[3] << 8) | data[2]);
    int16_t z = (int16_t)((data[5] << 8) | data[4]);

    x >>= 6;
    y >>= 6;
    z >>= 6;

    return {x, y, z};
}

lis3dh_g_t lis3dh_read_g()
{
    lis3dh_raw_t raw = lis3dh_read_raw();

    return {
        raw.x * 0.004f,
        raw.y * 0.004f,
        raw.z * 0.004f
    };
}

