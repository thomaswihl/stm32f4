#include "ws2801.h"

#include <cassert>

Ws2801::Ws2801(Spi<char> &spi, unsigned count) :
    mSpi(spi),
    mData(new uint8_t[count * 3]),
    mCount(count)
{
}

void Ws2801::enable()
{
    mSpi.config(Spi<char>::MasterSlave::Master, Spi<char>::ClockPolarity::LowWhenIdle, Spi<char>::ClockPhase::FirstTransition, Spi<char>::Endianess::MsbFirst);
    mSpi.setSpeed(1000000);
    mSpi.enable(Device::All);
}

void Ws2801::set(unsigned index, uint8_t red, uint8_t green, uint8_t blue)
{
    assert(index < mCount);
    mData[index * 3 + 0] = red;
    mData[index * 3 + 1] = green;
    mData[index * 3 + 2] = blue;
    mSpi.write(reinterpret_cast<char*>(mData), mCount * 3, nullptr);
}
