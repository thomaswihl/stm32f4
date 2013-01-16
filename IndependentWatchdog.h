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

#ifndef INDEPENDENTWATCHDOG_H
#define INDEPENDENTWATCHDOG_H

#include "System.h"

class IndependentWatchdog
{
public:
    IndependentWatchdog(System::BaseAddress base);

    void enable(uint32_t us);
    void service();

private:
    enum { MAX_RELOAD = 0xfff, CLOCK = 32000 };
    struct IWDG
    {
        uint16_t KR;
        uint16_t __RESERVED0;
        uint16_t PR : 3;
        uint16_t __RESERVED1 : 13;
        uint16_t __RESERVED2;
        uint16_t RLR : 12;
        uint16_t __RESERVED3 : 4;
        uint16_t __RESERVED4;
        struct __SR
        {
            uint16_t PVU : 1;
            uint16_t RVU : 1;
            uint16_t __RESERVED0 : 14;
        }   SR;
        uint16_t __RESERVED5;
    };
    volatile IWDG* mBase;
};

#endif // INDEPENDENTWATCHDOG_H
