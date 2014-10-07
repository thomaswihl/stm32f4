#include "i2c.h"

I2C::I2C(System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile IIC*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mTransferBuffer(64)
{
    static_assert(sizeof(IIC) == 0x24, "Struct has wrong size, compiler problem.");
}



void I2C::enable(Device::Part part)
{
}

void I2C::disable(Device::Part part)
{
}

bool I2C::transfer(I2C::Transfer *transfer)
{
    bool success = mTransferBuffer.push(transfer);
    //printf("PUSH\n", ((transfer->mReadData != nullptr) ? "R" : "-"), transfer->mReadData, ((transfer->mWriteData != nullptr) ? "W" : "-"), transfer->mWriteData, transfer->mLength);
    //if (mBase->CR2.RXDMAEN == 0 && mBase->CR2.TXDMAEN == 0) nextTransfer();
    return success;
}

void I2C::configDma(Dma::Stream *write, Dma::Stream *read)
{
}

void I2C::dmaReadComplete()
{
}

void I2C::dmaWriteComplete()
{
}

void I2C::configInterrupt(InterruptController::Line *interrupt)
{
}

void I2C::clockCallback(ClockControl::Callback::Reason reason, uint32_t clock)
{
}

void I2C::dmaCallback(Dma::Stream *stream, Dma::Stream::Callback::Reason reason)
{
}

void I2C::interruptCallback(InterruptController::Index index)
{
}

void I2C::setSpeed(uint32_t maxSpeed, Mode mode)
{
    uint32_t clock = mClockControl->clock(mClock);
    bool pe = mBase->CR1.PE;
    mBase->CR1.PE = 0;
    mBase->CR2.FREQ = (clock / 500000 + 1) / 2;
    switch (mode)
    {
    case Mode::Standard:
        mBase->CCR.CCR = std::max(4LU, (clock / maxSpeed + 1) / 2);
        break;
    case Mode::FastDuty2:
        mBase->CCR.CCR = std::max(1LU, (clock / maxSpeed + 2) / 3);
        break;
    case Mode::FastDuty16by9:
        mBase->CCR.CCR = std::max(1LU, (clock / maxSpeed + 24) / (9 + 16));
        break;

    }
    mBase->CR1.PE = pe;
}

void I2C::nextTransfer()
{
    Transfer* t;

    if (mTransferBuffer.back(t))
    {
        if ((t->mWriteLength == 0 || t->mWriteData == nullptr) && (t->mReadLength == 0 || t->mReadData == nullptr))
        {
            mTransferBuffer.pop(t);
            nextTransfer();
            return;
        }
        if (t->mChip != nullptr) t->mChip->prepare();
        setSpeed(t->mMaxSpeed, t->mMode);
        mBase->CCR.FS = t->mMode != Mode::Standard;
        mBase->CCR.DUTY = t->mMode == Mode::FastDuty16by9;
        if (mDmaWrite != nullptr && t->mWriteData != nullptr)
        {
            mBase->CR2.DMAEN = 1;
            mDmaWrite->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(t->mWriteData));
            mDmaWrite->setTransferCount(t->mWriteLength);
            mDmaWrite->start();
        }
        if (mDmaRead != nullptr && t->mReadData != nullptr)
        {
            mBase->CR2.DMAEN = 1;
            mDmaRead->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(t->mReadData));
            mDmaRead->setTransferCount(t->mReadLength);
            mDmaRead->start();
        }
    }
    else
    {
        mBase->CR2.DMAEN = 0;
    }
}


