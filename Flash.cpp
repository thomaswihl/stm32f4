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

#include <cassert>

const unsigned int Flash::SECTOR_COUNT;
const unsigned int Flash::SECTOR_SIZE[SECTOR_COUNT] =
{
     16 * 1024,
     16 * 1024,
     16 * 1024,
     16 * 1024,
     64 * 1024,
    128 * 1024,
    128 * 1024,
    128 * 1024,
    128 * 1024,
    128 * 1024,
    128 * 1024,
    128 * 1024,
};


Flash::Flash(System::BaseAddress base, ClockControl &clockControl, AccessSize accessSize) :
    mBase(reinterpret_cast<volatile FLASH*>(base)),
    mAccessSize(accessSize)
{
    static_assert(sizeof(FLASH) == 0x18, "Struct has wrong size, compiler problem.");
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

void Flash::unlock()
{
    unlockCr();
}

bool Flash::erase(unsigned int sector)
{
    mBase->CR.PSIZE = static_cast<uint32_t>(mAccessSize);
    mBase->CR.SER = 1;
    mBase->CR.SNB = sector;
    mBase->CR.STRT = 1;
    waitReady();
    mBase->CR.SER = 0;
    return result();
}

bool Flash::erase()
{
    mBase->CR.PSIZE = static_cast<uint32_t>(mAccessSize);
    mBase->CR.MER = 1;
    mBase->CR.STRT = 1;
    waitReady();
    mBase->CR.MER = 0;
    return result();
}

template<class T>
bool Flash::write(uint32_t address, const T *data, unsigned int count)
{
    uint32_t size = 0;
    while (sizeof(T) != (1 << size)) ++size;
    mBase->CR.PSIZE = size;
    volatile T* dest = reinterpret_cast<volatile T*>(address);
    mBase->CR.PG = 1;
    for (unsigned int i = 0; i < count; ++i) *dest++ = *data++;
    waitReady();
    mBase->CR.PG = 0;
    return result();
}

void Flash::lock()
{
    mBase->CR.LOCK = 1;
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

void Flash::unlockCr()
{
    mBase->KEYR = 0x45670123;
    mBase->KEYR = 0xCDEF89AB;
}

void Flash::unlockOptcr()
{
    mBase->OPTKEYR = 0x08192A3B;
    mBase->OPTKEYR = 0x4C5D6E7F;
}

void Flash::waitReady()
{
    while (mBase->SR.BSY == 1)
    {
    }
}

bool Flash::result()
{
    if (mBase->SR.PGSERR != 0 || mBase->SR.PGPERR != 0 || mBase->SR.PGAERR != 0 || mBase->SR.WRPERR != 0) return false;
    return true;
}
