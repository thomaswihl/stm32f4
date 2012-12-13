/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
    uint32_t clock = mClock->clock(ClockControl::Clock::AHB) / 8000;
    clock *= mInterval;
    mBase->CTRL.CLKSOURCE = 0;
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
