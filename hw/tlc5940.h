#ifndef TLC5940_H
#define TLC5940_H

#include "../Spi.h"
#include "../Timer.h"
#include "../System.h"

class Tlc5940 : public System::Event::Callback
{
public:
    Tlc5940(Spi& spi, Gpio::Pin& xlat, Gpio::Pin& blank, Timer& gsclkPwm, Timer& gsclkLatch);
    void setOutput(int index, int percent);
    void send();

private:
    static const int GRAYSCALE_DATA_COUNT = 16 + 8;
    Spi& mSpi;
    Gpio::Pin& mXlat;
    Gpio::Pin& mBlank;
    Timer& mPwm;
    Timer& mLatch;

    uint8_t* mGrayScaleData;
    System::Event mLatchEvent;
    System::Event mSpiEvent;
    bool mNewData;
    Spi::Transfer mTransfer;

    virtual void eventCallback(System::Event* event);

};

#endif // TLC5940_H
