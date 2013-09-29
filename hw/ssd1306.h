#ifndef SSD1306_H
#define SSD1306_H

#include "../Spi.h"
#include "../System.h"

class Ssd1306
{
public:
    Ssd1306(Spi& spi, Gpio::Pin& cs, Gpio::Pin& dataCommand, Gpio::Pin& reset);

    void reset();
    void init();

    void setPixel(int x, int y, bool on = true);
    void sendData();

private:
    static const int DISPLAY_WIDTH = 128;
    static const int DISPLAY_HEIGHT = 64;
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
        ComScanInc = 0xc0,
        ComScanDec = 0xc8,
        ComPins = 0xda,
        Contrast = 0x81,
        ChargePump = 0x8d,
        PreCharge = 0xd9,
        DeselectLevel = 0xdb,
        EntireDisplayOnResume = 0xa4,
        EntireDisplayOn = 0xa5,
        DisplayNormal = 0xa6,
        DisplayInverse = 0xa7,
        LowColumn0 = 0x00,
        HighColumn0 = 0x10,
    };

    Spi& mSpi;
    Gpio::Pin& mCs;
    Gpio::Pin& mDc;
    Gpio::Pin& mReset;
    Spi::Transfer mTransfer;

    char mFb[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

    void sendCommand(Command cmd);
    void sendCommand(uint8_t cmd);
};

#endif // SSD1306_H
