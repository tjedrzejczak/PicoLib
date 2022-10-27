/*
 * BMP280_I2C.cpp
 */

#include "BMP280_I2C.hpp"

#define REG_CONFIG _u(0xF5)
#define REG_CTRL_MEAS _u(0xF4)
#define REG_PRESSURE_MSB _u(0xF7)
#define REG_DIG_T1_LSB _u(0x88)

#define NUM_CALIB_PARAMS 24

//////////////////////////////////////////////////////////////
// Bosch Datasheet types
#define BMP280_S32_t int32_t
#define BMP280_U32_t uint32_t

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
// BMP280_S32_t t_fine;
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T, BMP280_compensation_t *c, BMP280_S32_t &t_fine)
{
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BMP280_S32_t)c->dig_T1 << 1))) * ((BMP280_S32_t)c->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BMP280_S32_t)c->dig_T1)) * ((adc_T >> 4) - ((BMP280_S32_t)c->dig_T1))) >> 12) * ((BMP280_S32_t)c->dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P, BMP280_compensation_t *c, BMP280_S32_t &t_fine)
{
    BMP280_S32_t var1, var2;
    BMP280_U32_t p;
    var1 = (((BMP280_S32_t)t_fine) >> 1) - (BMP280_S32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((BMP280_S32_t)c->dig_P6);
    var2 = var2 + ((var1 * ((BMP280_S32_t)c->dig_P5)) << 1);
    var2 = (var2 >> 2) + (((BMP280_S32_t)c->dig_P4) << 16);
    var1 = (((c->dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((BMP280_S32_t)c->dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((BMP280_S32_t)c->dig_P1)) >> 15);
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = (((BMP280_U32_t)(((BMP280_S32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000)
    {
        p = (p << 1) / ((BMP280_U32_t)var1);
    }
    else
    {
        p = (p / (BMP280_U32_t)var1) * 2;
    }
    var1 = (((BMP280_S32_t)c->dig_P9) * ((BMP280_S32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((BMP280_S32_t)(p >> 2)) * ((BMP280_S32_t)c->dig_P8)) >> 13;
    p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + c->dig_P7) >> 4));
    return p;
}
//////////////////////////////////////////////////////////////

BMP280Device::BMP280Device(I2CPort *port, int address)
{
    _port = port;
    _address = address;
}

void BMP280Device::Init()
{
    sendConfifuration();
    readCompensation();
}

BMP280_temperature_pressure_t BMP280Device::Read()
{
    BMP280_temperature_pressure_t ret;

    uint8_t buf[6];
    uint8_t reg = REG_PRESSURE_MSB;
    _port->WriteBlocking(_address, &reg, 1, true);
    _port->ReadBlocking(_address, buf, 6, false);

    BMP280_S32_t pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    BMP280_S32_t temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);

    BMP280_S32_t t_fine;
    ret.temperature = bmp280_compensate_T_int32(temp, &_compensation, t_fine);
    ret.pressure = bmp280_compensate_P_int32(pressure, &_compensation, t_fine);

    return ret;
}

void BMP280Device::sendConfifuration()
{
    const uint8_t config =
        ((0b101 << 5) | // Standby time 1000ms
         (0b100 << 2)   // Filter coefficient 16
        );

    uint8_t buf[2];
    buf[0] = REG_CONFIG;
    buf[1] = config;
    _port->WriteBlocking(_address, buf, 2, false);

    const uint8_t ctrl_meas =
        ((0b001 << 5) | // Temperature: 16 bit / 0.0050 °C
         (0b011 << 2) | // Pressure: Standard resolution 18 bit / 0.66 Pa
         (0b11)         // Power mode: Normal
        );

    buf[0] = REG_CTRL_MEAS;
    buf[1] = ctrl_meas;
    _port->WriteBlocking(_address, buf, 2, false);
}

void BMP280Device::readCompensation()
{
    uint8_t buf[NUM_CALIB_PARAMS] = {0};
    uint8_t reg = REG_DIG_T1_LSB;

    _port->WriteBlocking(_address, &reg, 1, true);
    _port->ReadBlocking(_address, buf, NUM_CALIB_PARAMS, false);

    _compensation.dig_T1 = (uint16_t)(buf[1] << 8) | buf[0];
    _compensation.dig_T2 = (int16_t)(buf[3] << 8) | buf[2];
    _compensation.dig_T3 = (int16_t)(buf[5] << 8) | buf[4];

    _compensation.dig_P1 = (uint16_t)(buf[7] << 8) | buf[6];
    _compensation.dig_P2 = (int16_t)(buf[9] << 8) | buf[8];
    _compensation.dig_P3 = (int16_t)(buf[11] << 8) | buf[10];
    _compensation.dig_P4 = (int16_t)(buf[13] << 8) | buf[12];
    _compensation.dig_P5 = (int16_t)(buf[15] << 8) | buf[14];
    _compensation.dig_P6 = (int16_t)(buf[17] << 8) | buf[16];
    _compensation.dig_P7 = (int16_t)(buf[19] << 8) | buf[18];
    _compensation.dig_P8 = (int16_t)(buf[21] << 8) | buf[20];
    _compensation.dig_P9 = (int16_t)(buf[23] << 8) | buf[22];
}