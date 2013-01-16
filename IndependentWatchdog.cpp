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

#include "IndependentWatchdog.h"

IndependentWatchdog::IndependentWatchdog(System::BaseAddress base) :
    mBase(reinterpret_cast<volatile IWDG*>(base))
{
    static_assert(sizeof(IWDG) == 0x10, "Struct has wrong size, compiler problem.");
}

void IndependentWatchdog::enable(uint32_t us)
{
    uint16_t prescaler = 0;
    // reload = us * clock / prescaler
    while (((us * CLOCK) >> (2 + prescaler)) >= MAX_RELOAD) ++prescaler;

    mBase->KR = 0x5555;
    mBase->PR = prescaler;  // 0 = /4, 1 = /8, 2 = /16, ...
    mBase->RLR = (us * CLOCK) >> (2 + prescaler);
    mBase->KR = 0xcccc;
}

void IndependentWatchdog::service()
{
    mBase->KR = 0xaaaa;
}
