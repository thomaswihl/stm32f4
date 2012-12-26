#include "Spi.h"

template<typename T>
Spi<T>::Spi(System &system, System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    Stream<T>(system),
    mBase(reinterpret_cast<volatile SPI*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mDmaTxComplete(*this),
    mDmaRxComplete(*this),
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
    br &= 7;
    mBase->CR1.BR = br;
    mSpeed = maxSpeed;
    return clock / (1 << (br + 1));
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
void Spi<T>::read(T *data, unsigned int count)
{
    if (readPrepare(data, count))
    {
        simpleRead();
    }
}

template<typename T>
void Spi<T>::read(T *data, unsigned int count, System::Event *callback)
{
    readPrepare(data, count);
}

template<typename T>
void Spi<T>::write(const T *data, unsigned int count)
{
    if (writePrepare(data, count))
    {
        select();
        simpleWrite();
        deselect();
    }
}

template<typename T>
void Spi<T>::write(const T *data, unsigned int count, System::Event *callback)
{
    if (writePrepare(data, count))
    {
        select();
        simpleWrite();
        deselect();
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
void Spi<T>::interruptCallback(InterruptController::Index index)
{
}

template<typename T>
void Spi<T>::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
}

template<typename T>
void Spi<T>::eventCallback(System::Event *event)
{
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
void Spi<T>::triggerWrite()
{
    if (mDmaTx != 0 && Stream<T>::mWriteData)
    {
//        mBase->CR3.DMAT = 1;
//        mDmaTx->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(mWriteData));
//        mDmaTx->setTransferCount(mWriteCount);
//        mBase->SR.TC = 0;
//        if (mInterrupt != nullptr) mBase->CR1.TCIE = 1;
//        mDmaTx->start();
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
        simpleWrite();
    }
}

template<typename T>
void Spi<T>::triggerRead()
{
    if (mDmaRx != 0)
    {
//        mBase->CR3.DMAR = 1;
//        mDmaRx->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(mReadData));
//        mDmaRx->setTransferCount(mReadCount);
//        mDmaRx->start();
    }
    else if (mInterrupt != 0)
    {
//        mBase->CR1.RXNEIE = 1;
    }
    else
    {
        // we have to do it manually
        simpleRead();
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
void Spi<T>::simpleRead()
{
    do
    {
        waitReceiveNotEmpty();
    }   while (Stream<T>::read(static_cast<T>(mBase->DR)));
    Stream<T>::readFinished(true);
}

template<typename T>
void Spi<T>::simpleWrite()
{
    T c;
    while (Stream<T>::write(c))
    {
        waitTransmitComplete();
        mBase->DR = c;
        waitReceiveNotEmpty();
        Stream<T>::read(static_cast<T>(mBase->DR));
    }
    Stream<T>::readFinished(true);
    Stream<T>::writeFinished(true);
}

template class Spi<char>;
template class Spi<uint16_t>;

