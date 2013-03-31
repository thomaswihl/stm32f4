#ifndef TLC5940_H
#define TLC5940_H

#include "../Spi.h"
#include "../Timer.h"
#include "../System.h"

class Tlc5940
{
public:
    Tlc5940(Spi<char>& spi, Gpio::Pin& xlat, Gpio::Pin& blank, Timer& gsclk);
    void setOutput(int index, uint16_t value);
    void send();

private:
    Spi<char>& mSpi;
    Gpio::Pin& mXlat;
    Gpio::Pin& mBlank;
    Timer& mGsClk;

    char mGrayScaleData[16 + 8];

};

#endif // TLC5940_H
