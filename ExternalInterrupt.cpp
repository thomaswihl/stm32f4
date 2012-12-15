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

#include <cstdio>

ExternalInterrupt::ExternalInterrupt(unsigned int base, std::size_t vectorSize) :
    mBase(reinterpret_cast<volatile EXTI*>(base))
{
    static_assert(sizeof(EXTI) == 0x18, "Struct has wrong size, compiler problem.");
    mCallback = new InterruptController::Callback*[vectorSize];
}

ExternalInterrupt::~ExternalInterrupt()
{
}

void ExternalInterrupt::set(InterruptController::Index index, InterruptController::Callback *callback)
{
    mCallback[index] = callback;
}


void ExternalInterrupt::handle(InterruptController::Index index)
{
    if (mCallback[index] != 0) mCallback[index]->interruptCallback(index);
}
