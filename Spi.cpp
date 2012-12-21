#include "Spi.h"

template<typename T>
Spi<T>::Spi(System &system, System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    Stream<T>(system),
    mBase(reinterpret_cast<volatile SPI*>(base)),
    mClockControl(clockControl),
    mClock(clock)
{
    static_assert(sizeof(SPI) == 0x24, "Struct has wrong size, compiler problem.");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2, "Only 8 and 16 bits are supported");
    mBase->CR1.DFF = (sizeof(T) == 1) ? 0 : 1;
}

template<typename T>
uint32_t Spi<T>::setSpeed(uint32_t maxSpeed)
{
    uint32_t clock = mClockControl->clock(mClock);
    uint32_t divider = (clock + maxSpeed / 2) / maxSpeed;
    uint32_t br = 0;
    while ((1u << (br + 1u)) < divider) ++br;
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
void Spi<T>::read(T *data, unsigned int count)
{
}

template<typename T>
void Spi<T>::read(T *data, unsigned int count, System::Event *callback)
{
}

template<typename T>
void Spi<T>::write(const T *data, unsigned int count)
{
}

template<typename T>
void Spi<T>::write(const T *data, unsigned int count, System::Event *callback)
{
}


template<typename T>
void Spi<T>::enable(Device::Part part)
{
    mBase->CR1.SPE = 1;
}

template<typename T>
void Spi<T>::disable(Device::Part part)
{
    mBase->CR1.SPE = 1;
}

template class Spi<char>;
template class Spi<uint16_t>;

