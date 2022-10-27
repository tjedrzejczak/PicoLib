/*
 * HD44780_I2C.cpp
 */

#include <stdarg.h>
#include "HD44780_I2C.hpp"

constexpr uint8_t HD44780_COMMAND_CLEARDISPLAY = 0x01;

constexpr uint8_t HD44780_COMMAND_CURSORHOME = 0x02;

constexpr uint8_t HD44780_COMMAND_ENTRYMODESET = 0x04;
constexpr uint8_t HD44780_ENTRYMODE_SHIFT_CURSOR = 0x00;
constexpr uint8_t HD44780_ENTRYMODE_SHIFT_DISPLAY = 0x01;
constexpr uint8_t HD44780_ENTRYMODE_DECREMENT = 0x00;
constexpr uint8_t HD44780_ENTRYMODE_INCREMENT = 0x02;

constexpr uint8_t HD44780_COMMAND_DISPLAYCONTROL = 0x08;
constexpr uint8_t HD44780_DISPLAYCONTROL_CURSOR_NOBLINK = 0x00;
constexpr uint8_t HD44780_DISPLAYCONTROL_CURSOR_BLINK = 0x01;
constexpr uint8_t HD44780_DISPLAYCONTROL_CURSOR_OFF = 0x00;
constexpr uint8_t HD44780_DISPLAYCONTROL_CURSOR_ON = 0x02;
constexpr uint8_t HD44780_DISPLAYCONTROL_OFF = 0x00;
constexpr uint8_t HD44780_DISPLAYCONTROL_ON = 0x04;

constexpr uint8_t HD44780_COMMAND_DISPLAYCURSORSHIFT = 0x10;
constexpr uint8_t HD44780_DISPLAYCURSORSHIFT_LEFT = 0x00;
constexpr uint8_t HD44780_DISPLAYCURSORSHIFT_RIGHT = 0x04;
constexpr uint8_t HD44780_DISPLAYCURSORSHIFT_CURSOR = 0x00;
constexpr uint8_t HD44780_DISPLAYCURSORSHIFT_DISPLAY = 0x08;

constexpr uint8_t HD44780_COMMAND_FUNCTIONSET = 0x20;
constexpr uint8_t HD44780_FUNCTIONSET_FONT5x7 = 0x00;
constexpr uint8_t HD44780_FUNCTIONSET_FONT5x10 = 0x04;
constexpr uint8_t HD44780_FUNCTIONSET_ONE_LINE = 0x00;
constexpr uint8_t HD44780_FUNCTIONSET_TWO_LINE = 0x08;
constexpr uint8_t HD44780_FUNCTIONSET_4BIT = 0x00;
constexpr uint8_t HD44780_FUNCTIONSET_8BIT = 0x01;

constexpr uint8_t HD44780_BACKLIGHT = 0x08;

constexpr uint8_t HD44780_CGRAM_SET = 0x40;
constexpr uint8_t HD44780_DDRAM_SET = 0x80;

constexpr uint8_t HD44780_ENABLE_BIT = 0x04;

HD44780Device::HD44780Device(I2CPort *port, int address)
{
    _port = port;
    _address = address;
}

// public

void HD44780Device::Init()
{
    _backlight = HD44780_BACKLIGHT;

    for (int i = 0; i < 3; i++)
    {
        sendCommand(0x03);
        sleep_ms(5);
    }
    sendCommand(0x02);
    sleep_ms(5);

    sendCommand(HD44780_COMMAND_DISPLAYCONTROL);
    sendCommand(HD44780_COMMAND_ENTRYMODESET | HD44780_ENTRYMODE_SHIFT_CURSOR | HD44780_ENTRYMODE_INCREMENT);
    sendCommand(HD44780_COMMAND_FUNCTIONSET | HD44780_FUNCTIONSET_FONT5x7 | HD44780_FUNCTIONSET_TWO_LINE);
    sendCommand(HD44780_COMMAND_DISPLAYCONTROL | HD44780_DISPLAYCONTROL_CURSOR_OFF | HD44780_DISPLAYCONTROL_ON);

    Clear();
}

void HD44780Device::Clear()
{
    sendCommand(HD44780_COMMAND_CLEARDISPLAY);
}

void HD44780Device::GoTo(uint8_t x, uint8_t y)
{
    if (y > 1)
    {
        y = y % 2;
        x += 20;
    }

    sendCommand(HD44780_DDRAM_SET | (x + (0x40 * y)));
}

void HD44780Device::PrintText(const char *text)
{
    while (*text)
        sendData(*text++);
}

void HD44780Device::PrintTextFormat(uint8_t col, uint8_t row, const char *format, ...)
{
    static char msgBuffer[16];

    va_list args;
    va_start(args, format);
    vsprintf(msgBuffer, format, args);
    va_end(args);

    GoTo(col, row);
    PrintText(msgBuffer);
}

// private

void HD44780Device::sendCommand(uint8_t command)
{
    uint8_t high = (command & 0xF0) | _backlight;
    uint8_t low = (command << 4) & 0xF0 | _backlight;

    _port->WriteByte(_address, high);
    toggleEnable(high);
    _port->WriteByte(_address, low);
    toggleEnable(low);
}

void HD44780Device::sendData(uint8_t command)
{
    uint8_t high = (command & 0xF0) | 0x01 | _backlight;
    uint8_t low = (command << 4) & 0xF0 | 0x01 | _backlight;

    _port->WriteByte(_address, high);
    toggleEnable(high);
    _port->WriteByte(_address, low);
    toggleEnable(low);
}

void HD44780Device::toggleEnable(uint8_t val)
{
    constexpr int delay = 600;

    sleep_us(delay);
    _port->WriteByte(_address, val | HD44780_ENABLE_BIT);
    sleep_us(delay);
    _port->WriteByte(_address, val & ~HD44780_ENABLE_BIT);
    sleep_us(delay);
}