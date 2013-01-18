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

#include "SysCfg.h"

SysCfg::SysCfg(System::BaseAddress base) :
    mBase(reinterpret_cast<volatile SYSCFG*>(base))
{
    static_assert(sizeof(SYSCFG) == 0x24, "Struct has wrong size, compiler problem.");
}

void SysCfg::extIntSource(::Gpio::Index index, SysCfg::Gpio gpio)
{
    unsigned int i = static_cast<unsigned int>(index);
    unsigned int shift = (i & 3) << 2;
    i >>= 2;
    mBase->EXTICR[i] = (mBase->EXTICR[i] & ~(0xf << shift)) | (static_cast<uint32_t>(gpio) << shift);
}
