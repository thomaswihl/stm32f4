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

#include "Serial.h"

#include <cassert>
#include <cstdio>

Serial::Serial(System &system, System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    mSystem(system),
    mBase(reinterpret_cast<volatile USART*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mSpeed(0),
    mReadBuffer(READ_BUFFER_SIZE),
    mWriteBuffer(WRITE_BUFFER_SIZE)
{
    static_assert(sizeof(USART) == 0x1c, "Struct has wrong size, compiler problem.");
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

void Serial::configDma(Dma::Stream *tx, Dma::Stream *rx)
{
    if (mDmaTx != 0)
    {
        mDmaTx->waitReady();
        delete mDmaTx;
    }
    if (mDmaRx != 0)
    {
        mDmaRx->waitReady();
        delete mDmaRx;
    }
    mDmaTx = tx;
    mDmaRx = rx;
}

void Serial::configInterrupt(InterruptController::Line* interrupt)
{
    if (mInterrupt != nullptr) delete mInterrupt;
    mBase->CR1.TCIE = 0;
    mBase->CR1.RXNEIE = 1;
    mInterrupt = interrupt;
    if (mInterrupt != nullptr)
    {
        mInterrupt->setHandler(this);
        mInterrupt->enable();
    }
}

int Serial::read(char* data, int size)
{
    while (mReadBuffer.used() == 0)
    {
    }

    return mReadBuffer.read(data, size);
}

int Serial::write(const char *data, int size)
{
    if (mDmaTx != 0)
    {

    }
    else if (mInterrupt != 0)
    {
        mWriteBuffer.write(data, size);
        if (!mBase->CR1.TCIE)
        {
            mBase->CR1.TCIE = 1;
            mBase->DR = *data++;
        }
    }
    else
    {
        for (int i = 0; i < size; ++i)
        {
            while (!mBase->SR.TC)
            {
            }
            mBase->DR = *data++;
        }
    }
    return size;
}

void Serial::handle(InterruptController::Index index)
{
    if (mBase->SR.RXNE)
    {
        mReadBuffer.push(mBase->DR);
        mSystem.postEvent(std::shared_ptr<System::Event>(new Event(reinterpret_cast<System::BaseAddress>(mBase), Event::Type::ReceivedByte)));
    }
    if (mBase->SR.TC)
    {
        char c;
        if (mWriteBuffer.pop(c)) mBase->DR = c;
        else mBase->CR1.TCIE = 0;
    }
}

void Serial::clockPrepareChange(uint32_t newClock)
{
}

void Serial::clockChanged(uint32_t newClock)
{
    if (mSpeed != 0) setSpeed(mSpeed);
}

