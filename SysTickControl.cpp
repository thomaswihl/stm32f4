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


SysTickControl::SysTickControl(System::BaseAddress base, ClockControl *clock) :
    mBase(reinterpret_cast<volatile STK*>(base)),
    mClock(clock),
    mSingleCountTime(1),
    mCountPerMs(1),
    mMilliseconds(0),
    mNextTick(-1)
{
    static_assert(sizeof(STK) == 0x10, "Struct has wrong size, compiler problem.");
    clock->addChangeHandler(this);
    config();
    setNextTick(1000);
}

void SysTickControl::enable()
{
    mBase->VAL = 0;
    mBase->CTRL.ENABLE = 1;
}

void SysTickControl::disable()
{
    mBase->CTRL.ENABLE = 0;
}

void SysTickControl::setNextTick(unsigned ms)
{
    disable();
    mBase->RELOAD = ms * mCountPerMs - 1;
    mNextTick = ms;
    enable();
}

void SysTickControl::addRepeatingEvent(SysTickControl::RepeatingEvent *event)
{
    if (event != nullptr)
    {
        mRepeatingEvents.push_back(event);
    }
}

void SysTickControl::removeRepeatingEvent(SysTickControl::RepeatingEvent *event)
{
    if (event != nullptr)
    {

        //mRepeatingEvents.erase();
    }
}

// IRQ callback
void SysTickControl::tick()
{
    unsigned nextTick = 1000;
    mMilliseconds += mNextTick;
    for (auto& iter : mRepeatingEvents)
    {
        iter->millisecondsPassed(mNextTick);
        if (iter->msRemain() < nextTick) nextTick = iter->msRemain();
    }
    setNextTick(nextTick);
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
    uint32_t ms;
    uint32_t count;
    do
    {
        mBase->CTRL.COUNTFLAG = 0;
        count = mBase->VAL;
        ms = mMilliseconds;
    }   while (mBase->CTRL.COUNTFLAG);
    uint64_t val = mBase->RELOAD - count;
    val *= mSingleCountTime;
    val += ms * static_cast<uint64_t>(1000000);
    return val;
}

void SysTickControl::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed) config();
}

void SysTickControl::config()
{
    mCountPerMs = mClock->clock(ClockControl::Clock::AHB) / 8000;
    mSingleCountTime = 1000000 / mCountPerMs;
    mBase->CTRL.CLKSOURCE = 0;
    mBase->CTRL.TICKINT = 1;
}



void SysTickControl::RepeatingEvent::millisecondsPassed(unsigned ms)
{
    mMsFromStart += ms;
    if (mMsFromStart >= mMs)
    {
        System::instance()->postEvent(this);
        mMsFromStart -= mMs;
        if (mMsFromStart >= mMs) mMsFromStart = 0;
    }
}
