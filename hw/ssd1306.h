#ifndef SSD1306_H
#define SSD1306_H

#include "../Spi.h"
#include "../System.h"

class Ssd1306 : public System::Event::Callback, public Spi::ChipSelect
{
public:
    Ssd1306(Spi& spi, Gpio::Pin& cs, Gpio::Pin& dataCommand, Gpio::Pin& reset);

    void reset();
    void init();

    void clear();
    void setPixel(int x, int y, bool on = true);
    void drawString(int x, int y, const char *string);
    void drawChar(int x, int y, char c);
    void sendData();

private:
    static const int DISPLAY_WIDTH = 128;
    static const int DISPLAY_HEIGHT = 64;
    static const int FB_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8;
    enum Command
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
    System::Event mSpiEvent;
    Spi::Transfer mTransfer;

    uint8_t* mFb;

    void sendCommands(const uint8_t *cmds, unsigned size);

    // ChipSelect interface
public:
    void select();
    void deselect();

    // Callback interface
public:
    void eventCallback(System::Event *event);
};

#endif // SSD1306_H
