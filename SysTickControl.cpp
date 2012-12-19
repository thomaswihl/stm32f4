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
    mInterval(msInterval),
    mSingleCountTime(1)
{
    static_assert(sizeof(STK) == 0x10, "Struct has wrong size, compiler problem.");
    clock->addChangeHandler(this);
}

void SysTickControl::enable()
{
    uint32_t clock = mClock->clock(ClockControl::Clock::AHB) / 8000;
    mSingleCountTime = 1000000000 / clock;
    clock *= mInterval;
    mBase->CTRL.CLKSOURCE = 0;
    mBase->RELOAD = clock - 1;
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
    uint64_t start = ns() / 1000;
    uint64_t current = start;
    do
    {
        current = ns() / 1000;
    }   while (current - start < us);
}

uint64_t SysTickControl::ns()
{
    uint64_t val;
    do
    {
        mBase->CTRL.COUNTFLAG = 0;
        val = mBase->RELOAD - mBase->VAL;
        val *= mSingleCountTime;
        val += System::ticks() * mInterval * static_cast<uint64_t>(1000000);
    }   while (mBase->CTRL.COUNTFLAG);
    return val;
}

void SysTickControl::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::AboutToChange) disable();
    else enable();
}

