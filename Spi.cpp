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
