#include "SysTickControl.h"


SysTickControl::SysTickControl(System::BaseAddress base, ClockControl *clock, unsigned int msInterval) :
    mBase(reinterpret_cast<volatile STK*>(base)),
    mClock(clock),
    mInterval(msInterval)
{
    static_assert(sizeof(STK) == 0x10, "Struct has wrong size, compiler problem.");
    clock->addChangeHandler(this);
}

void SysTickControl::enable()
{
    uint32_t clock = mClock->clock(ClockControl::Clock::System) / 1000;
    clock *= mInterval;
    mBase->RELOAD = clock;
    mBase->VAL = 0;
    mBase->CTRL.TICKINT = 1;
    mBase->CTRL.ENABLE = 1;
}

void SysTickControl::disable()
{
    mBase->CTRL.TICKINT = 0;
    mBase->CTRL.ENABLE = 0;
}

void SysTickControl::usleep(unsigned int us)
{
    unsigned int start = System::ticks();
    unsigned int current = start;
    us /= 1000;
    do
    {
        __asm("wfi");
        current = System::ticks();
    }   while (current - start < us);
}

void SysTickControl::clockPrepareChange(uint32_t newClock)
{
    disable();
}

void SysTickControl::clockChanged(uint32_t newClock)
{
    enable();
}
