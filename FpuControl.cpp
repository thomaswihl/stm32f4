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

#include "FpuControl.h"

FpuControl::FpuControl(System::BaseAddress base) :
    mBase(reinterpret_cast<volatile FPU*>(base))
{
    static_assert(sizeof(FPU) == 0x10, "Struct has wrong size, compiler problem.");
}

void FpuControl::enable(FpuControl::AccessPrivileges privileges)
{
    mBase->CPACR.CP = static_cast<uint32_t>(privileges);
    __asm("DSB");
    __asm("ISB");
}
