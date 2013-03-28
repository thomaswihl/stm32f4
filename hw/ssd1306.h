#ifndef SSD1306_H
#define SSD1306_H

#include "../Spi.h"
#include "../System.h"

class Ssd1306
{
public:
    Ssd1306(Spi<char>& spi, Gpio::Pin& cs, Gpio::Pin& dataCommand, Gpio::Pin& reset);

    void init();

private:
    enum class Command : uint8_t
    {
        DisplayOn = 0xaf,
        DisplayOff = 0xae,
        ClockDivide = 0xd5,
        Multiplex = 0xa8,
        DisplayOffset = 0xd3,
        StartLine0 = 0x40,
        MemoryAddressing = 0x20,
        SegRemap0 = 0xa0,
        SegRemap127 = 0xa1,
        ComScanInc = 0xd0,
        ComScanDec = 0xd8,
        ComPins = 0xda,
        Contrast = 0x81,
        ChargePump = 0x8d,
        PreCharge = 0xd9,
        DeselectLevel = 0xdb,
        EntireDisplayOn = 0xa4,
        EntireDisplayOnResume = 0xa5,
        DisplayNormal = 0xa6,
        DisplayInverse = 0xa7,
        LowColumn0 = 0x00,
        HighColumn0 = 0x10,
    };

    Spi<char>& mSpi;
    Gpio::Pin& mCs;
    Gpio::Pin& mDc;
    Gpio::Pin& mReset;

    char mFb[128 * 64 / 8];

    void sendCommand(Command cmd);
    void sendCommand(uint8_t cmd);
    void sendData();
};

#endif // SSD1306_H
