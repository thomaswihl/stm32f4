#include "Spi.h"

template<typename T>
Spi<T>::Spi(System &system, System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    Stream<T>(system),
    mBase(reinterpret_cast<volatile SPI*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mChipSelect(nullptr),
    mActiveLow(true)
{
    static_assert(sizeof(SPI) == 0x24, "Struct has wrong size, compiler problem.");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2, "Only 8 and 16 bits are supported");
    mBase->CR1.DFF = (sizeof(T) == 1) ? 0 : 1;
}

template<typename T>
uint32_t Spi<T>::setSpeed(uint32_t maxSpeed)
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

template<typename T>
void Spi<T>::setMasterSlave(Spi::MasterSlave masterSlave)
{
    mBase->CR1.MSTR = static_cast<uint32_t>(masterSlave);
}

template<typename T>
void Spi<T>::setClock(Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase)
{
    mBase->CR1.CPOL = static_cast<uint32_t>(clockPolarity);
    mBase->CR1.CPHA = static_cast<uint32_t>(clockPhase);
}

template<typename T>
void Spi<T>::setEndianess(Spi::Endianess endianess)
{
    mBase->CR1.LSBFIRST = static_cast<uint32_t>(endianess);
}

template<typename T>
void Spi<T>::config(Spi::MasterSlave masterSlave, Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase, Spi::Endianess endianess)
{
    setMasterSlave(masterSlave);
    setClock(clockPolarity, clockPhase);
    setEndianess(endianess);
}

template<typename T>
void Spi<T>::configChipSelect(Gpio::Pin *chipSelect, bool activeLow)
{
    mChipSelect = chipSelect;
    mActiveLow = activeLow;
    deselect();
    if (mChipSelect != nullptr)
    {
        mBase->CR1.SSM = 1;
        mBase->CR1.SSI = 1;
    }
    else
    {
        mBase->CR1.SSM = 0;
        mBase->CR1.SSI = 0;
    }
}

template<typename T>
void Spi<T>::enable(Device::Part part)
{
    mBase->CR1.SPE = 1;
}

template<typename T>
void Spi<T>::disable(Device::Part part)
{
    mBase->CR1.SPE = 0;
}

template<typename T>
void Spi<T>::configDma(Dma::Stream *write, Dma::Stream *read)
{
    Device::configDma(write, read);
    Dma::Stream::DataSize dataSize = Dma::Stream::DataSize::Byte;
    if (sizeof(T) == 2) dataSize = Dma::Stream::DataSize::HalfWord;
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

template<typename T>
void Spi<T>::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed) setSpeed(mSpeed);
}

template<typename T>
void Spi<T>::interruptCallback(InterruptController::Index index)
{
}

template<typename T>
void Spi<T>::dmaReadComplete(bool success)
{
    mBase->CR2.RXDMAEN = 0;
    Stream<T>::readSuccess(success);
}

template<typename T>
void Spi<T>::dmaWriteComplete(bool success)
{
    mBase->CR2.TXDMAEN = 0;
    Stream<T>::writeSuccess(success);
}

template<typename T>
void Spi<T>::select()
{
    if (mChipSelect != nullptr)
    {
        if (mActiveLow) mChipSelect->reset();
        else mChipSelect->set();
    }
}

template<typename T>
void Spi<T>::deselect()
{
    if (mChipSelect != nullptr)
    {
        if (mActiveLow) mChipSelect->set();
        else mChipSelect->reset();
    }
}

template<typename T>
void Spi<T>::waitTransmitComplete()
{
    int timeout = 100000;
    while (!mBase->SR.TXE && timeout > 0)
    {
        --timeout;
    }
}

template<typename T>
void Spi<T>::waitReceiveNotEmpty()
{
    int timeout = 100000;
    while (!mBase->SR.RXNE && timeout > 0)
    {
        --timeout;
    }
}

template<typename T>
void Spi<T>::readPrepare()
{
    select();
}

template<typename T>
void Spi<T>::readSync()
{
    do
    {
        mBase->DR = 0;
        waitReceiveNotEmpty();
    }   while (Stream<T>::read(static_cast<T>(mBase->DR)));
}

template<typename T>
void Spi<T>::readTrigger()
{
    return;
    // empty the receive register before starting another transfer,
    // as it might be full from last transfer, in case it was a write only transfer
    T c;
    while (mBase->SR.RXNE) c = mBase->DR;
    (void)c;
    if (mDmaRead != 0)
    {
        mBase->CR2.RXDMAEN = 1;
        mDmaRead->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(Stream<T>::readData()));
        mDmaRead->setTransferCount(Stream<T>::readCount());
        mDmaRead->start();
    }
    else if (mInterrupt != 0)
    {
//        mBase->CR1.RXNEIE = 1;
    }
    else
    {
        // we have to do it manually
        readSync();
    }
}

template<typename T>
void Spi<T>::readDone()
{
    deselect();
}


template<typename T>
void Spi<T>::writePrepare()
{
    select();
}

template<typename T>
void Spi<T>::writeSync()
{
    T c;
    while (Stream<T>::write(c))
    {
        waitTransmitComplete();
        mBase->DR = c;
        waitReceiveNotEmpty();
        c = static_cast<T>(mBase->DR);
        Stream<T>::read(c);
    }
}

template<typename T>
void Spi<T>::writeTrigger()
{
    if (mDmaWrite != 0)
    {
        mBase->CR2.TXDMAEN = 1;
        mDmaWrite->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(Stream<T>::writeData()));
        mDmaWrite->setTransferCount(Stream<T>::writeCount());
        mDmaWrite->start();
    }
    else if (mInterrupt != 0)
    {
//        mBase->CR1.TCIE = 1;
//        // send first bye, to start transmission
//        char c;
//        if (Stream<char>::write(c)) mBase->DR = c;
//        else mBase->CR1.TCIE = 0;
    }
    else
    {
        // we have to do it manually
        writeSync();
    }
}

template<typename T>
void Spi<T>::writeDone()
{
    deselect();
}

template class Spi<char>;
template class Spi<uint16_t>;
