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

#include "InterruptController.h"

InterruptController::InterruptController(unsigned int base, std::size_t vectorSize) :
    mBase(reinterpret_cast<volatile NVIC*>(base))
{
    static_assert(sizeof(NVIC) == 0xe04, "Struct has wrong size, compiler problem.");
    mHandler = new Callback*[vectorSize];
}

InterruptController::~InterruptController()
{
}

void InterruptController::handle(Index index)
{
    if (mHandler[index] != 0) mHandler[index]->interruptCallback(index);
}

void InterruptController::setPriotity(InterruptController::Index index, InterruptController::Priority priority)
{
    mBase->IPR[index] = static_cast<uint8_t>(priority) << 4;
}

InterruptController::Line::Line(InterruptController &interruptController, InterruptController::Index index) :
    mInterruptController(interruptController),
    mIndex(index)
{
}

InterruptController::Line::~Line()
{
    disable();
    setCallback(0);
}

void InterruptController::Line::setCallback(InterruptController::Callback *handler)
{
    mInterruptController.mHandler[mIndex] = handler;
}

void InterruptController::Line::enable()
{
    mInterruptController.mBase->ISER[mIndex / 32] = 1 << (mIndex % 32);
}

void InterruptController::Line::disable()
{
    mInterruptController.mBase->ICER[mIndex / 32] = 1 << (mIndex % 32);
}
