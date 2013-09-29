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


//void Spi::setClock(Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase)
//{
//    mBase->CR1.CPOL = static_cast<uint32_t>(clockPolarity);
//    mBase->CR1.CPHA = static_cast<uint32_t>(clockPhase);
//}


//void Spi::setEndianess(Spi::Endianess endianess)
//{
//    mBase->CR1.LSBFIRST = static_cast<uint32_t>(endianess);
//}


//void Spi::config(Spi::MasterSlave masterSlave, Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase, Spi::Endianess endianess)
//{
//    setMasterSlave(masterSlave);
//    setClock(clockPolarity, clockPhase);
//    setEndianess(endianess);
//}



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
    return mTransferBuffer.push(transfer);
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
    //readDmaComplete(mDmaRead->transferCount());
}


void Spi::dmaWriteComplete()
{
    mBase->CR2.TXDMAEN = 0;
    //writeDmaComplete(mDmaWrite->transferCount());
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


//void Spi::readSync()
//{
//    do
//    {
//        mBase->DR = 0;
//        waitReceiveNotEmpty();
//    }   while (Stream<T>::read(static_cast<T>(mBase->DR)));
//}


//void Spi::readTrigger()
//{
//    return;
//    // empty the receive register before starting another transfer,
//    // as it might be full from last transfer, in case it was a write only transfer
//    T c;
//    while (mBase->SR.RXNE) c = mBase->DR;
//    (void)c;
//    if (mDmaRead != 0)
//    {
//        T* data;
//        unsigned int len;
//        readDmaBuffer(data, len);
//        if (len > 0)
//        {
//            mBase->CR2.RXDMAEN = 1;
//            mDmaRead->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(data));
//            mDmaRead->setTransferCount(len);
//            mDmaRead->start();
//        }
//    }
//    else if (mInterrupt != 0)
//    {
////        mBase->CR1.RXNEIE = 1;
//    }
//    else
//    {
//        // we have to do it manually
//        readSync();
//    }
//}


//void Spi::readDone()
//{
//    deselect();
//}



//void Spi::writePrepare()
//{
//    select();
//}


//void Spi::writeSync()
//{
//    T c;
//    while (Stream<T>::write(c))
//    {
//        waitTransmitComplete();
//        mBase->DR = c;
//        waitReceiveNotEmpty();
//        c = static_cast<T>(mBase->DR);
//        Stream<T>::read(c);
//    }
//}


//void Spi::writeTrigger()
//{
//    if (mDmaWrite != 0)
//    {
//        const T* data;
//        unsigned int len;
//        writeDmaBuffer(data, len);
//        if (len > 0)
//        {
//            mBase->CR2.TXDMAEN = 1;
//            mDmaWrite->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(data));
//            mDmaWrite->setTransferCount(len);
//            mDmaWrite->start();
//        }
//    }
//    else if (mInterrupt != 0)
//    {
////        mBase->CR1.TCIE = 1;
////        // send first bye, to start transmission
////        char c;
////        if (Stream<char>::write(c)) mBase->DR = c;
////        else mBase->CR1.TCIE = 0;
//    }
//    else
//    {
//        // we have to do it manually
//        writeSync();
//    }
//}


//void Spi::writeDone()
//{
//    deselect();
//}

