/*
 * (c) 2013 Thomas Wihl
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

#ifndef SYSCFG_H
#define SYSCFG_H

#include "System.h"
#include "Gpio.h"

#include "stdint.h"

class SysCfg
{
public:
    enum class Gpio { A, B, C, D, E, F, G, H, I };
    SysCfg(System::BaseAddress base);
    void extIntSource(::Gpio::Index index, Gpio gpio);

private:
    struct SYSCFG
    {
        struct __MEMRMP
        {
            uint32_t MEM_MODE : 2;
            uint32_t __RESERVED0 : 30;
        }   MEMRMP;
        struct __PMC
        {
            uint32_t __RESERVED0 : 23;
            uint32_t MII_RMII_SEL : 1;
            uint32_t __RESERVED1 : 8;
        }   PMC;
        uint32_t EXTICR[4];
        uint32_t __RESERVED0[2];
        struct __CMPCR
        {
            uint32_t CMP_PD : 1;
            uint32_t __RESERVED0 : 7;
            uint32_t READY : 1;
            uint32_t __RESERVED1 : 23;
        }   CMPCR;
    };
    volatile SYSCFG* mBase;
};

#endif // SYSCFG_H
