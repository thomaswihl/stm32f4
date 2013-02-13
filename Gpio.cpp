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

#include "Gpio.h"

Gpio::Gpio(unsigned int base) :
    mBase(reinterpret_cast<volatile GPIO*>(base))
{
    static_assert(sizeof(GPIO) == 0x28, "Struct has wrong size, compiler problem.");
}

Gpio::~Gpio()
{
}

bool Gpio::get(Index index)
{
    return (((mBase->IDR >> static_cast<int>(index)) & 1) != 0);
}

void Gpio::set(Gpio::Index index)
{
    mBase->BSRR.BS = (1 << static_cast<int>(index));
}

void Gpio::set(uint16_t indices)
{
    mBase->BSRR.BS = indices;
}

void Gpio::setValue(uint16_t value)
{
    mBase->ODR = value;
}

void Gpio::reset(Gpio::Index index)
{
    mBase->BSRR.BR = (1 << static_cast<int>(index));
}

void Gpio::reset(uint16_t indices)
{
    mBase->BSRR.BR = indices;
}

void Gpio::setMode(Gpio::Index index, Gpio::Mode mode)
{
    uint32_t i = static_cast<int>(index) * 2;
    mBase->MODER = (mBase->MODER & ~(3 << i)) | (static_cast<uint32_t>(mode) << i);
}

void Gpio::setOutputType(Gpio::Index index, Gpio::OutputType outputType)
{
    uint32_t i = static_cast<int>(index);
    mBase->OTYPER = (mBase->OTYPER & ~(1 << i)) | (static_cast<uint32_t>(outputType) << i);
}

void Gpio::setSpeed(Gpio::Index index, Gpio::Speed speed)
{
    uint32_t i = static_cast<int>(index) * 2;
    mBase->OSPEEDR = (mBase->OSPEEDR & ~(3 << i)) | (static_cast<uint32_t>(speed) << i);
}

void Gpio::setPull(Gpio::Index index, Gpio::Pull pull)
{
    uint32_t i = static_cast<int>(index) * 2;
    mBase->PUPDR = (mBase->PUPDR & ~(3 << i)) | (static_cast<uint32_t>(pull) << i);
}

void Gpio::setAlternate(Gpio::Index index, Gpio::AltFunc altFunc)
{
    int i = static_cast<int>(index);
    if (i < 8) mBase->AFRL = (mBase->AFRL & ~(0xf << i * 4)) | (static_cast<uint32_t>(altFunc) << (i * 4));
    else mBase->AFRH = (mBase->AFRH & ~(0xf << (i- 8) * 4)) | (static_cast<uint32_t>(altFunc) << ((i - 8) * 4));
    setMode(index, Mode::Alternate);
}

void Gpio::configInput(Gpio::Index index, Gpio::Pull pull)
{
    setMode(index, Mode::Input);
    setPull(index, pull);
}

void Gpio::configOutput(Gpio::Index index, Gpio::OutputType outputType, Pull pull, Gpio::Speed speed)
{
    setMode(index, Mode::Output);
    setOutputType(index, outputType);
    setPull(index, pull);
    setSpeed(index, speed);
}


Gpio::Pin::Pin(Gpio &gpio, Gpio::Index index) :
    mGpio(gpio),
    mIndex(index)
{
}

void Gpio::Pin::set(bool set)
{
    if (set) mGpio.set(mIndex);
    else mGpio.reset(mIndex);
}

void Gpio::Pin::reset()
{
    mGpio.reset(mIndex);
}

bool Gpio::Pin::get()
{
    return mGpio.get(mIndex);
}

