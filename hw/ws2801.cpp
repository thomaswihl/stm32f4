#include "ws2801.h"

#include <cassert>

Ws2801::Ws2801(Spi::Chip& spi, unsigned count) :
    mSpi(spi),
    mData(new uint8_t[count * 3]),
    mCount(count)
{
    memset(mData, 0, mCount * 3);
    mData[0] = 0xf0;
    mData[1] = 0xcc;
    mData[2] = 0xaa;

    memset(&mTransfer, 0, sizeof(mTransfer));
    mTransfer.mMaxSpeed = 2 * 1000 * 1000;
    mTransfer.mChipSelect = 0;
    mTransfer.mClockPhase = Spi::ClockPhase::FirstTransition;
    mTransfer.mClockPolarity = Spi::ClockPolarity::LowWhenIdle;
    mTransfer.mEndianess = Spi::Endianess::MsbFirst;
    //mTransfer.mEvent = 0;
    mTransfer.mWriteData = mData;
    mTransfer.mLength = mCount * 3;
}

void Ws2801::enable()
{
    mSpi.transfer(&mTransfer);
}

void Ws2801::set(unsigned index, uint8_t red, uint8_t green, uint8_t blue)
{
    assert(index < mCount);
    mData[index * 3 + 0] = red;
    mData[index * 3 + 1] = green;
    mData[index * 3 + 2] = blue;
    mSpi.transfer(&mTransfer);
}
