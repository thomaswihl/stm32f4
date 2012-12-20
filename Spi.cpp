#include "Spi.h"

Spi::Spi(System::BaseAddress base, System::Event::Component component, ClockControl *clockControl, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile SPI*>(base)),
    mComponent(component),
    mClockControl(clockControl),
    mClock(clock)
{
    static_assert(sizeof(SPI) == 0x24, "Struct has wrong size, compiler problem.");
}

uint32_t Spi::setSpeed(uint32_t maxSpeed)
{
    uint32_t clock = mClockControl->clock(mClock);
    uint32_t divider = (clock + maxSpeed / 2) / maxSpeed;
    uint32_t br = 0;
    while ((1 << (br + 1)) < divider) ++br;
    br &= 7;
    mBase->CR1.BR = br;
    mSpeed = maxSpeed;
    return clock / (1 << (br + 1));
}

void Spi::setMasterSlave(Spi::MasterSlave masterSlave)
{
    mBase->CR1.MSTR = static_cast<uint32_t>(masterSlave);
}

void Spi::setClock(Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase)
{
    mBase->CR1.CPOL = static_cast<uint32_t>(clockPolarity);
    mBase->CR1.CPHA = static_cast<uint32_t>(clockPhase);
}

void Spi::setDataWidth(Spi::DataWidth dataWidth)
{
    mBase->CR1.DFF = static_cast<uint32_t>(dataWidth);
}

void Spi::setEndianess(Spi::Endianess endianess)
{
    mBase->CR1.LSBFIRST = static_cast<uint32_t>(endianess);
}

void Spi::config(Spi::MasterSlave masterSlave, Spi::ClockPolarity clockPolarity, Spi::ClockPhase clockPhase, Spi::DataWidth dataWidth, Spi::Endianess endianess)
{
    setMasterSlave(masterSlave);
    setClock(clockPolarity, clockPhase);
    setDataWidth(dataWidth);
    setEndianess(endianess);
}


void Spi::enable()
{
    mBase->CR1.SPE = 1;
}

void Spi::disable()
{
    mBase->CR1.SPE = 1;
}
