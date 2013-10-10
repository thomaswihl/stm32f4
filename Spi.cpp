#include "Spi.h"


Spi::Spi(System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile SPI*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mTransferBuffer(64)
{
    static_assert(sizeof(SPI) == 0x24, "Struct has wrong size, compiler problem.");
    //mBase->CR1.DFF = (sizeof(T) == 1) ? 0 : 1;
}



void Spi::setMasterSlave(Spi::MasterSlave masterSlave)
{
    switch (masterSlave)
    {
    case Spi::MasterSlave::Master:
        mBase->CR1.SSM = 1;
        mBase->CR1.SSI = 1;
        mBase->CR1.MSTR = 1;
        break;
    case Spi::MasterSlave::Slave:
        mBase->CR1.SSM = 1;
        mBase->CR1.SSI = 0;
        mBase->CR1.MSTR = 1;
        break;
    case Spi::MasterSlave::MasterNssOut:
        mBase->CR1.SSM = 0;
        mBase->CR2.SSOE = 1;
        mBase->CR1.MSTR = 1;
        break;
    case Spi::MasterSlave::MasterNssIn:
        mBase->CR1.SSM = 0;
        mBase->CR2.SSOE = 0;
        mBase->CR1.MSTR = 1;
        break;
    }
}

void Spi::enable(Device::Part part)
{
    mBase->CR1.SPE = 1;
}

void Spi::disable(Device::Part part)
{
    mBase->CR1.SPE = 0;
}

bool Spi::transfer(Transfer *transfer)
{
    bool success = mTransferBuffer.push(transfer);
    if (mBase->CR2.RXDMAEN == 0 && mBase->CR2.TXDMAEN == 0) nextTransfer();
    return success;
}

void Spi::nextTransfer()
{
    Transfer* t;

    if (mTransferBuffer.back(t))
    {
        if (t->mLength == 0)
        {
            mTransferBuffer.pop(t);
            nextTransfer();
            return;
        }
        //printf("SPI %s(%08x)%s(%08x) %i bytes\n", ((t->mReadData != nullptr) ? "R" : ""), t->mReadData, ((t->mWriteData != nullptr) ? "W" : ""), t->mWriteData, t->mLength);
        if (t->mChip != nullptr) t->mChip->prepare();
        if (t->mChipSelect != nullptr) t->mChipSelect->select();
        setSpeed(t->mMaxSpeed);
        config(t->mClockPolarity, t->mClockPhase, t->mEndianess);
        if (mDmaRead != nullptr && t->mReadData != nullptr)
        {
            mBase->CR2.RXDMAEN = 1;
            mDmaRead->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(t->mReadData));
            mDmaRead->setTransferCount(t->mLength);
            mDmaRead->start();
        }
        if (mDmaWrite != nullptr && t->mWriteData != nullptr)
        {
//            for (int i = 0; i < t->mLength; ++i) printf("%02x ", t->mWriteData[i]);
//            printf("\n");
            mBase->CR2.TXDMAEN = 1;
            mDmaWrite->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(t->mWriteData));
            mDmaWrite->setTransferCount(t->mLength);
            mDmaWrite->start();
        }
    }
}

void Spi::writeSync()
{
    Transfer* t;
    if (mTransferBuffer.pop(t))
    {
        unsigned len = t->mLength;
        const uint8_t* w = t->mWriteData;
        uint8_t* r = t->mReadData;
        while (len > 0)
        {
            waitTransmitComplete();
            mBase->DR = *w++;
            waitReceiveNotEmpty();
            *r++ = static_cast<uint8_t>(mBase->DR);
            --len;
        }
    }
}


void Spi::configDma(Dma::Stream *write, Dma::Stream *read)
{
    Device::configDma(write, read);
    Dma::Stream::DataSize dataSize = Dma::Stream::DataSize::Byte;
    if (Device::mDmaWrite != nullptr)
    {
        mDmaWrite->config(Dma::Stream::Direction::MemoryToPeripheral, false, true, dataSize, dataSize, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaWrite->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaWrite->configFifo(Dma::Stream::FifoThreshold::Quater);
    }
    if (Device::mDmaRead != nullptr)
    {
        mDmaRead->config(Dma::Stream::Direction::PeripheralToMemory, false, true, dataSize, dataSize, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaRead->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaWrite->configFifo(Dma::Stream::FifoThreshold::Quater);
    }
}


void Spi::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed) setSpeed(mSpeed);
}


void Spi::interruptCallback(InterruptController::Index index)
{
}


void Spi::dmaReadComplete()
{
    mBase->CR2.RXDMAEN = 0;
    Transfer* t;
    if (mTransferBuffer.pop(t))
    {
        if (t->mChipSelect != nullptr) t->mChipSelect->deselect();
        if (t->mEvent != nullptr) System::instance()->postEvent(t->mEvent);
        nextTransfer();
    }
}


void Spi::dmaWriteComplete()
{
    mBase->CR2.TXDMAEN = 0;
    Transfer* t;
    if (mTransferBuffer.back(t))
    {
        if (t->mReadData == nullptr)
        {
            mTransferBuffer.pop(t);
            if (t->mChipSelect != nullptr) t->mChipSelect->deselect();
            if (t->mEvent != nullptr) System::instance()->postEvent(t->mEvent);
            nextTransfer();
        }
    }
}

void Spi::waitTransmitComplete()
{
    int timeout = 100000;
    while (!mBase->SR.TXE && timeout > 0)
    {
        --timeout;
    }
}


void Spi::waitReceiveNotEmpty()
{
    int timeout = 100000;
    while (!mBase->SR.RXNE && timeout > 0)
    {
        --timeout;
    }
}

uint32_t Spi::setSpeed(uint32_t maxSpeed)
{
    uint32_t clock = mClockControl->clock(mClock);
    uint32_t divider = (clock + maxSpeed - 1) / maxSpeed;
    uint32_t br = 0;
    while ((2u << br) < divider) ++br;
    if (br > 7) br = 7;
    mBase->CR1.BR = br;
    mSpeed = maxSpeed;
    return clock / (2 << br);
}

void Spi::config(Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase, Spi::Endianess endianess)
{
    mBase->CR1.SPE = 0;
    mBase->CR1.CPOL = static_cast<uint32_t>(clockPolarity);
    mBase->CR1.CPHA = static_cast<uint32_t>(clockPhase);
    mBase->CR1.LSBFIRST = static_cast<uint32_t>(endianess);
    mBase->CR1.SPE = 1;
}

