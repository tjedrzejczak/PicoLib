/*
 * I2C.hpp
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdio.h>
#include "hardware/i2c.h"

class I2CPort
{
public:
    I2CPort(i2c_inst_t *channel, uint baudrate, uint gpioSDA, uint gpioSCL);
    void Init();
    int WriteBlocking(uint8_t address, const uint8_t *src, size_t len, bool nostop);
    int ReadBlocking(uint8_t address, uint8_t *dst, size_t len, bool nostop);

private:
    i2c_inst_t *_channel;
    uint _baudrate;
    uint _gpioSDA;
    uint _gpioSCL;
};

#endif /* I2C_H_ */