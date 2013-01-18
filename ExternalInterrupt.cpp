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

#include "ExternalInterrupt.h"

#include "System.h"

#include <cstdio>

ExternalInterrupt::ExternalInterrupt(unsigned int base, unsigned int vectorSize) :
    mBase(reinterpret_cast<volatile EXTI*>(base)),
    mVectorSize(vectorSize)
{
    static_assert(sizeof(EXTI) == 0x18, "Struct has wrong size, compiler problem.");
    mCallback = new InterruptController::Callback*[vectorSize];
}

ExternalInterrupt::~ExternalInterrupt()
{
}

void ExternalInterrupt::interruptCallback(InterruptController::Index index)
{
    uint32_t pr = mBase->PR;
    for (unsigned int i = 0; i < mVectorSize; ++i)
    {
        if ((pr & (1 << i)) != 0)
        {
            if (mCallback[i] != 0)
            {
                mCallback[i]->interruptCallback(i);
            }
            else
            {
                printf("Unhandled %i\n", i);
                System::instance()->printError("EXTI", "Unhandled Interrupt");
                mBase->IMR = mBase->IMR & ~(1 << i);
            }
        }
    }
    // clear pending interrupts
    mBase->PR = pr;
}

ExternalInterrupt::Line::Line(ExternalInterrupt &interruptController, InterruptController::Index index) :
    mInterruptController(interruptController),
    mIndex(index)
{
}

ExternalInterrupt::Line::~Line()
{
}

void ExternalInterrupt::Line::setCallback(InterruptController::Callback *handler)
{
    mInterruptController.mCallback[mIndex] = handler;
}

void ExternalInterrupt::Line::enable(Trigger trigger)
{
    uint32_t bit = 1 << mIndex;
    uint32_t rom = 0;
    uint32_t ram = 0xffffffff;
    uint32_t fom = 0;
    uint32_t fam = 0xffffffff;

    switch (trigger)
    {
//    case Trigger::Level:
//        ram &= ~bit;
//        fam &= ~bit;
//        break;
    case Trigger::Rising:
        rom |= bit;
        fam &= ~bit;
        break;
    case Trigger::Falling:
        ram &= ~bit;
        fom |= bit;
        break;
    case Trigger::RisingAndFalling:
        rom |= bit;
        fom |= bit;
        break;
    }
    mInterruptController.mBase->RTSR = (mInterruptController.mBase->RTSR & ram) | rom;
    mInterruptController.mBase->FTSR = (mInterruptController.mBase->FTSR & fam) | fom;

    mInterruptController.mBase->IMR = mInterruptController.mBase->IMR | (1 << mIndex);
}

void ExternalInterrupt::Line::disable()
{
    mInterruptController.mBase->IMR = mInterruptController.mBase->IMR & ~(1 << mIndex);
}

InterruptController::Index ExternalInterrupt::Line::index()
{
    return mIndex;
}


