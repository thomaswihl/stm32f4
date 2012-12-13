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

class Flash : public ClockControl::ChangeHandler
{
public:
    enum class Feature
    {
        InstructionCache,
        DataCache,
        Prefetch,
    };

    Flash(System::BaseAddress base, ClockControl& clockControl);
    virtual ~Flash();
    void set(Feature feature, bool enable);
protected:
    virtual void clockPrepareChange(uint32_t newClock);
    virtual void clockChanged(uint32_t newClock);

private:
    struct FLASH
    {
        struct __ACR
        {
            uint16_t LATENCY : 3;
            uint16_t __RESERVED0 : 5;
            uint16_t PRFTEN : 1;
            uint16_t ICEN : 1;
            uint16_t DCEN : 1;
            uint16_t ICRST : 1;
            uint16_t DCRST : 1;
            uint16_t __RESERVED1 : 3;
        }   ACR;
        uint16_t __RESERVED0;
    };
    volatile FLASH* mBase;

    uint16_t getWaitStates(uint32_t clock);
};

#endif // FLASH_H
