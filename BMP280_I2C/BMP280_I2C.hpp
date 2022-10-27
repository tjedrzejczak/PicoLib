/*
 * BMP280_I2C.hpp
 */

#ifndef BMP280_I2C_H_
#define BMP280_I2C_H_

#include <stdio.h>
#include "../I2C/I2C.hpp"

typedef struct BMP280_compensation
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
} BMP280_compensation_t;

typedef struct BMP280_temperature_pressure
{
    int32_t temperature;
    uint32_t pressure;
} BMP280_temperature_pressure_t;

class BMP280Device
{
public:
    BMP280Device(I2CPort *port, int address);
    void Init();
    BMP280_temperature_pressure_t Read();

private:
    I2CPort *_port;
    int _address;
    BMP280_compensation_t _compensation;

    void sendConfifuration();
    void readCompensation();
};

#endif /* BMP280_I2C_H_ */