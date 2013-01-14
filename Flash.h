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

#ifndef FLASH_H
#define FLASH_H

#include "ClockControl.h"

class Flash : public ClockControl::Callback
{
public:
    enum class Feature
    {
        InstructionCache,
        DataCache,
        Prefetch,
    };
    enum class AccessSize { x8 = 0, x16 = 1, x32 = 2, x64 = 3 };

    Flash(System::BaseAddress base, ClockControl& clockControl, AccessSize accessSize);
    virtual ~Flash();
    void set(Feature feature, bool enable);
    void unlock();
    bool erase(unsigned int sector);
    bool erase();
    template<class T>
    bool write(uint32_t address, const T* data, unsigned int count);
    void lock();
protected:
    virtual void clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock);

private:
    struct FLASH
    {
        struct __ACR
        {
            uint32_t LATENCY : 3;
            uint32_t __RESERVED0 : 5;
            uint32_t PRFTEN : 1;
            uint32_t ICEN : 1;
            uint32_t DCEN : 1;
            uint32_t ICRST : 1;
            uint32_t DCRST : 1;
            uint32_t __RESERVED1 : 3;
        }   ACR;
        uint32_t KEYR;
        uint32_t OPTKEYR;
        struct __SR
        {
            uint32_t EOP : 1;
            uint32_t OPERR : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t WRPERR : 1;
            uint32_t PGAERR : 1;
            uint32_t PGPERR : 1;
            uint32_t PGSERR : 1;
            uint32_t __RESERVED1 : 8;
            uint32_t BSY : 1;
            uint32_t __RESERVED2 : 15;
        }   SR;
        struct __CR
        {
            uint32_t PG : 1;
            uint32_t SER : 1;
            uint32_t MER : 1;
            uint32_t SNB : 4;
            uint32_t __RESERVED0 : 1;
            uint32_t PSIZE : 2;
            uint32_t __RESERVED1 : 6;
            uint32_t STRT : 1;
            uint32_t __RESERVED2 : 7;
            uint32_t EOPIE : 1;
            uint32_t ERRIE : 1;
            uint32_t __RESERVED3 : 5;
            uint32_t LOCK : 1;
        }   CR;
        struct __OPTCR
        {
            uint32_t OPTLOCK : 1;
            uint32_t OPTSTRT : 1;
            uint32_t BOR_LEV : 2;
            uint32_t __RESERVED0 : 1;
            uint32_t WDG_SW : 1;
            uint32_t nRST_STOP : 1;
            uint32_t nRST_STDBY : 1;
            uint32_t RDP : 8;
            uint32_t nWRP : 12;
            uint32_t __RESERVED1 : 4;
        }   OPTCR;
    };
    static const unsigned int SECTOR_COUNT = 12;
    static const unsigned int SECTOR_SIZE[SECTOR_COUNT];
    volatile FLASH* mBase;
    AccessSize mAccessSize;

    uint32_t getWaitStates(uint32_t clock);
    void unlockCr();
    void unlockOptcr();
    void waitReady();
    bool result();
};

#endif // FLASH_H
