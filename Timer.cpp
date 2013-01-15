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

#include "Timer.h"

Timer::Timer(System::BaseAddress base) :
    mBase(reinterpret_cast<volatile TIMER*>(base))
{
    static_assert(sizeof(TIMER) == 0x54, "Struct has wrong size, compiler problem.");
}

void Timer::enable(Part part)
{
    mBase->CR1.CEN = 1;
}

void Timer::disable(Part part)
{
    mBase->CR1.CEN = 0;
}

void Timer::dmaReadComplete(bool success)
{
}

void Timer::dmaWriteComplete(bool success)
{
}

void Timer::setCounter(uint32_t counter)
{
    mBase->CNT = counter;
}

uint32_t Timer::counter()
{
    return mBase->CNT;
}

void Timer::setPrescaler(uint16_t prescaler)
{
    mBase->PSC = prescaler;
}

uint16_t Timer::prescaler()
{
    return mBase->PSC;
}

void Timer::setReload(uint32_t reload)
{
    mBase->ARR = reload;
}

uint32_t Timer::reload()
{
    return mBase->ARR;
}

void Timer::interruptCallback(InterruptController::Index index)
{
}
