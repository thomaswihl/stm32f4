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

#include "Flash.h"

Flash::Flash(System::BaseAddress base, ClockControl &clockControl) :
    mBase(reinterpret_cast<volatile FLASH*>(base))
{
    static_assert(sizeof(FLASH) == 0x4, "Struct has wrong size, compiler problem.");
    clockControl.addChangeHandler(this);
}

Flash::~Flash()
{
}

void Flash::set(Flash::Feature feature, bool enable)
{
    switch (feature)
    {
    case Feature::InstructionCache: mBase->ACR.ICEN = enable; break;
    case Feature::DataCache: mBase->ACR.DCEN = enable; break;
    case Feature::Prefetch: mBase->ACR.PRFTEN = enable; break;
    }
}

void Flash::clockCallback(Reason reason, uint32_t newClock)
{
    uint32_t ws = getWaitStates(newClock);
    if (mBase->ACR.LATENCY < ws) mBase->ACR.LATENCY = ws;
}

uint32_t Flash::getWaitStates(uint32_t clock)
{
    if (clock <= 30000000) return 0;
    if (clock <= 60000000) return 1;
    if (clock <= 90000000) return 2;
    if (clock <= 120000000) return 3;
    if (clock <= 150000000) return 4;
    if (clock <= 168000000) return 5;
    return 6;
}
