/*
 * HD44780_I2C.hpp
 */

#ifndef HD44780_I2C_H_
#define HD44780_I2C_H_

#include "../I2C/I2C.hpp"

class HD44780Device
{
public:
    HD44780Device(I2CPort *port, int address);
    void Init();
    void Clear();
    void GoTo(uint8_t x, uint8_t y);
    void PrintText(const char *text);
    void PrintTextFormat(uint8_t col, uint8_t row, const char *format, ...);

private:
    I2CPort *_port;
    int _address;
    uint8_t _backlight;

    void sendCommand(uint8_t byte);
    void sendData(uint8_t byte);
    void toggleEnable(uint8_t val);
};

#endif /* HD44780_I2C_H_ */