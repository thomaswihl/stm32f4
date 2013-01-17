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
    mSingleCountTime(1),
    mEvent(nullptr)
{
    static_assert(sizeof(STK) == 0x10, "Struct has wrong size, compiler problem.");
    clock->addChangeHandler(this);
}

void SysTickControl::enable()
{
    mBase->CTRL.CLKSOURCE = 0;
    mBase->VAL = 0;
    mBase->CTRL.TICKINT = 1;
    mBase->CTRL.ENABLE = 1;
}

void SysTickControl::disable()
{
    mBase->CTRL.TICKINT = 0;
    mBase->CTRL.ENABLE = 0;
}

void SysTickControl::setInterval(unsigned int msInterval)
{
    mInterval = msInterval;
    config();
}

unsigned int SysTickControl::interval()
{
    return mInterval;
}

void SysTickControl::tick()
{
    ++mTicks;
    if (mEvent != nullptr) System::postEvent(mEvent);
}

unsigned int SysTickControl::ticks()
{
    return mTicks;
}

void SysTickControl::setEvent(System::Event *event)
{
    mEvent = event;
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
    uint32_t ticks;
    uint32_t count;
    do
    {
        mBase->CTRL.COUNTFLAG = 0;
        count = mBase->VAL;
        ticks = mTicks;
    }   while (mBase->CTRL.COUNTFLAG);
    uint64_t val = mBase->RELOAD - count;
    val *= mSingleCountTime;
    val += ticks * mInterval * static_cast<uint64_t>(1000000);
    return val;
}

void SysTickControl::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed) config();
}

void SysTickControl::config()
{
    uint32_t clock = mClock->clock(ClockControl::Clock::AHB) / 8000;
    mSingleCountTime = 1000000 / clock;
    clock *= mInterval;
    mBase->RELOAD = clock - 1;
}

