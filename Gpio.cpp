#include "Gpio.h"

Gpio::Gpio(unsigned int base) :
    mBase(reinterpret_cast<volatile GPIO*>(base))
{
}

Gpio::~Gpio()
{
}

bool Gpio::get(Pin index)
{
    return (((mBase->IDR >> static_cast<int>(index)) & 1) != 0);
}

void Gpio::set(Gpio::Pin index)
{
    mBase->ODR |= (1 << static_cast<int>(index));
}

void Gpio::set(uint16_t indices)
{
    mBase->BSRR.BS = indices;
}

void Gpio::setValue(uint16_t value)
{
    mBase->ODR = value;
}

void Gpio::reset(Gpio::Pin index)
{
    mBase->ODR &= ~(1 << static_cast<int>(index));
}

void Gpio::reset(uint16_t indices)
{
    mBase->BSRR.BR = indices;
}

void Gpio::setMode(Gpio::Pin index, Gpio::Mode mode)
{
    uint32_t i = static_cast<int>(index) * 2;
    mBase->MODER = (mBase->MODER & ~(3 << i)) | (static_cast<uint32_t>(mode) << i);
}

void Gpio::setOutputType(Gpio::Pin index, Gpio::OutputType outputType)
{
    uint32_t i = static_cast<int>(index);
    mBase->OTYPER = (mBase->OTYPER & ~(1 << i)) | (static_cast<uint32_t>(outputType) << i);
}

void Gpio::setSpeed(Gpio::Pin index, Gpio::Speed speed)
{
    uint32_t i = static_cast<int>(index) * 2;
    mBase->OSPEEDR = (mBase->OSPEEDR & ~(3 << i)) | (static_cast<uint32_t>(speed) << i);
}

void Gpio::setPull(Gpio::Pin index, Gpio::Pull pull)
{
    uint32_t i = static_cast<int>(index) * 2;
    mBase->PUPDR = (mBase->PUPDR & ~(3 << i)) | (static_cast<uint32_t>(pull) << i);
}

void Gpio::setAlternate(Gpio::Pin index, Gpio::AltFunc altFunc)
{
    int i = static_cast<int>(index);
    if (i < 8) mBase->AFRL = (mBase->AFRL & ~(0xf << i * 4)) | (static_cast<uint32_t>(altFunc) << (i * 4));
    else mBase->AFRH = (mBase->AFRH & ~(0xf << (i- 8) * 4)) | (static_cast<uint32_t>(altFunc) << ((i - 8) * 4));
    setMode(index, Mode::Alternate);
}

void Gpio::configInput(Gpio::Pin index, Gpio::Pull pull)
{
    setMode(index, Mode::Input);
    setPull(index, pull);
}

void Gpio::configOutput(Gpio::Pin index, Gpio::OutputType outputType, Pull pull, Gpio::Speed speed)
{
    setMode(index, Mode::Output);
    setOutputType(index, outputType);
    setPull(index, pull);
    setSpeed(index, speed);
}


