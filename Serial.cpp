#include "Serial.h"

#include <cassert>
#include <cstdio>

Serial::Serial(System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile USART*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mSpeed(0)
{
    clockControl->addChangeHandler(this);
}

Serial::~Serial()
{
    mBase->SR.NF = 1;
    mBase->DR = 2;
    mBase->CR1.SBK = 1;

}

void Serial::setSpeed(uint32_t speed)
{
    uint32_t clock = mClockControl->clock(mClock);
    uint32_t accuracy = 8 * (2 - mBase->CR1.OVER8);
    uint32_t divider = clock / speed;
    mBase->BRR.DIV_MANTISSA = divider / accuracy;
    mBase->BRR.DIV_FRACTION = divider % accuracy;
    mSpeed = speed;
}

void Serial::setWordLength(Serial::WordLength dataBits)
{
    mBase->CR1.M = static_cast<uint32_t>(dataBits);
}

void Serial::setParity(Serial::Parity parity)
{
    mBase->CR1.PCE = (static_cast<uint32_t>(parity) & 2) >> 1;
    mBase->CR1.PS = static_cast<uint32_t>(parity) & 1;
}

void Serial::setStopBits(Serial::StopBits stopBits)
{
    mBase->CR2.STOP = static_cast<uint32_t>(stopBits);
}

void Serial::setHardwareFlowControl(Serial::HardwareFlowControl hardwareFlow)
{
    mBase->CR3.CTSE = hardwareFlow == HardwareFlowControl::Cts || hardwareFlow == HardwareFlowControl::CtsRts;
    mBase->CR3.RTSE = hardwareFlow == HardwareFlowControl::Rts || hardwareFlow == HardwareFlowControl::CtsRts;
}

void Serial::enable(bool enable)
{
    mBase->CR1.UE = enable;
    mBase->CR1.TE = enable;
    mBase->CR1.RE = enable;
}

void Serial::config(uint32_t speed, Serial::WordLength dataBits, Serial::Parity parity, Serial::StopBits stopBits, HardwareFlowControl hardwareFlow)
{
    enable(false);
    setSpeed(speed);
    setWordLength(dataBits);
    setParity(parity);
    setStopBits(stopBits);
    setHardwareFlowControl(hardwareFlow);
    enable(true);
}

void Serial::read(System::Buffer &buffer)
{
}

void Serial::write(System::Buffer &buffer)
{
    char* data = buffer.data();
    for (unsigned int i = 0; i < buffer.size(); ++i)
    {
        mBase->DR = *data++;
        while (!mBase->SR.TC)
        {
        }
    }
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
    if (mSpeed != 0) setSpeed(mSpeed);
}

