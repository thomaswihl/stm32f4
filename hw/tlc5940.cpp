#include "tlc5940.h"

Tlc5940::Tlc5940(Spi<char> &spi, Gpio::Pin &xlat, Gpio::Pin &blank, Timer &gsclk) :
    mSpi(spi),
    mXlat(xlat),
    mBlank(blank),
    mGsClk(gsclk)
{
    std::memset(mGrayScaleData, 0, sizeof(mGrayScaleData));
    mSpi.config(Spi<char>::MasterSlave::Master, Spi<char>::ClockPolarity::LowWhenIdle, Spi<char>::ClockPhase::FirstTransition, Spi<char>::Endianess::MsbFirst);
    mSpi.setSpeed(10 * 1000* 1000);
    mSpi.enable(Device::All);
    mBlank.set();
    mXlat.reset();
}

// 00 01 11 22 23 33 44 45 55

void Tlc5940::setOutput(int index, uint16_t value)
{
    if (index < 0 || index > 15) return;

    int i = 15 - index;

    if ((i % 2) == 0)
    {
        mGrayScaleData[i + i / 2] = value >> 4;
        mGrayScaleData[i + i / 2 + 1] = (mGrayScaleData[i + i / 2 + 1] & 0x0f) | ((value << 4) & 0xf0);
    }
    else
    {
        mGrayScaleData[i + i / 2] = (mGrayScaleData[i + i / 2 + 1] & 0xf0) | ((value >> 8) & 0x0f);
        mGrayScaleData[i + i / 2 + 1] = value;
    }
}

void Tlc5940::send()
{
    mSpi.write(mGrayScaleData, sizeof(mGrayScaleData));
    mBlank.set();
    System::instance()->nspin(100);
    mXlat.set();
    System::instance()->nspin(100);
    mXlat.reset();
    System::instance()->nspin(100);
    mBlank.reset();
}
