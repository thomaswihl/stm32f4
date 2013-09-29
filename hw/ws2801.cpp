#include "ws2801.h"

#include <cassert>

Ws2801::Ws2801(Spi &spi, unsigned count) :
    mSpi(spi),
    mData(new uint8_t[count * 3]),
    mCount(count)
{
}

void Ws2801::enable()
{
    memset(&mTransfer, 0, sizeof(mTransfer));
    mTransfer.maxSpeed = 25 * 1000* 1000;
    mTransfer.mChipSelect = 0;
    mTransfer.mClockPhase = Spi::ClockPhase::FirstTransition;
    mTransfer.mClockPolarity = Spi::ClockPolarity::LowWhenIdle;
    mTransfer.mEndianess = Spi::Endianess::MsbFirst;
    //mTransfer.mEvent = 0;
    mTransfer.mWriteData = mData;
    mTransfer.mWriteDataCount = mCount * 3;
}

void Ws2801::set(unsigned index, uint8_t red, uint8_t green, uint8_t blue)
{
    assert(index < mCount);
    mData[index * 3 + 0] = red;
    mData[index * 3 + 1] = green;
    mData[index * 3 + 2] = blue;
    mSpi.transfer(&mTransfer);
}
