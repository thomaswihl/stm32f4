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
    mHandler = new InterruptController::Handler*[vectorSize];
}

ExternalInterrupt::~ExternalInterrupt()
{
}

void ExternalInterrupt::set(InterruptController::Index index, InterruptController::Handler *handler)
{
    mHandler[index] = handler;
}


void ExternalInterrupt::handle(InterruptController::Index index)
{
    if (mHandler[index] != 0) mHandler[index]->handle(index);
}
