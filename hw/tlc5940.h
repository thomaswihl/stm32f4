#ifndef TLC5940_H
#define TLC5940_H

#include "../Spi.h"
#include "../Timer.h"
#include "../System.h"

class Tlc5940 : public System::Event::Callback
{
public:
    Tlc5940(Spi<char>& spi, Gpio::Pin& xlat, Gpio::Pin& blank, Timer& gsclkPwm, Timer& gsclkLatch);
    void setOutput(int index, uint16_t value);
    void send();

private:
    static const int GRAYSCALE_DATA_COUNT = 16 + 8;
    Spi<char>& mSpi;
    Gpio::Pin& mXlat;
    Gpio::Pin& mBlank;
    Timer& mPwm;
    Timer& mLatch;

    char* mGrayScaleData;
    System::Event mLatchEvent;
    System::Event mSpiEvent;
    bool mNewData;

    virtual void eventCallback(System::Event* event);

};

#endif // TLC5940_H
