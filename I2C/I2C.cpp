/*
 * I2C.cpp
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "I2C.hpp"

I2CPort::I2CPort(i2c_inst_t *channel, uint baudrate, uint gpioSDA, uint gpioSCL)
{
    _channel = channel;
    _baudrate = baudrate;
    _gpioSDA = gpioSDA;
    _gpioSCL = gpioSCL;
}

void I2CPort::Init()
{
    i2c_init(_channel, _baudrate);
    gpio_set_function(_gpioSDA, GPIO_FUNC_I2C);
    gpio_set_function(_gpioSCL, GPIO_FUNC_I2C);
    gpio_pull_up(_gpioSDA);
    gpio_pull_up(_gpioSCL);
}

int I2CPort::WriteBlocking(uint8_t address, const uint8_t *src, size_t len, bool nostop)
{
    return i2c_write_blocking(_channel, address, src, len, nostop);
}

int I2CPort::ReadBlocking(uint8_t address, uint8_t *dst, size_t len, bool nostop)
{
    return i2c_read_blocking(_channel, address, dst, len, nostop);
}

#include "I2C.hpp"