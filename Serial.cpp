#include "Serial.h"

#include "stm32f4xx_usart.h"

#include <cassert>
#include <cstdio>

Serial::Serial(System::BaseAddress base, ClockControl &clockControl) :
    mBase(reinterpret_cast<volatile USART*>(base))
{
    clockControl.addChangeHandler(this);
}

Serial::~Serial()
{
    mBase->SR.NF = 1;
    mBase->DR = 2;
    mBase->CR1.SBK = 1;

}

void Serial::setBaudrate(uint32_t baud)
{

}

void Serial::read(System::Buffer &buffer)
{
}

void Serial::write(System::Buffer &buffer)
{
}

void Serial::handle(Interrupt::Index index)
{
    if (mBase->SR.RXNE)
    {
        char c = mBase->DR;
        if (c == '\r') std::putchar('\n');
        else std::putchar(c);
    }
}

void Serial::clockPrepareChange(uint32_t newClock)
{
}

void Serial::clockChanged(uint32_t newClock)
{
}

