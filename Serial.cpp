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

Serial::Serial(System &system, System::BaseAddress base, System::Event::Component component, ClockControl *clockControl, ClockControl::Clock clock) :
    mSystem(system),
    mBase(reinterpret_cast<volatile USART*>(base)),
    mComponent(component),
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
    if (mDmaTx != nullptr) delete mDmaTx;
    if (mDmaRx != nullptr) delete mDmaRx;
    mBase->CR3.DMAT = 0;
    mBase->CR3.DMAR = 0;
    mDmaTx = tx;
    if (mDmaTx != nullptr)
    {
        mDmaTx->configure(Dma::Stream::Direction::MemoryToPeripheral, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaTx->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mBase->CR3.DMAT = 1;
        mBase->CR1.TCIE = 0;
        mDmaTx->setCallback(this);
    }
    mDmaRx = rx;
    if (mDmaRx != nullptr)
    {
        mDmaRx->configure(Dma::Stream::Direction::PeripheralToMemory, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaRx->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mBase->CR3.DMAR = 1;
        mBase->CR1.RXNEIE = 0;
        mDmaRx->setCallback(this);
    }
}

void Serial::configInterrupt(InterruptController::Line* interrupt)
{
    if (mInterrupt != nullptr) delete mInterrupt;
    mBase->CR1.TCIE = 0;
    mBase->CR1.RXNEIE = 0;
    mInterrupt = interrupt;
    if (mInterrupt != nullptr)
    {
        mBase->CR1.RXNEIE = (mDmaRx == nullptr) ? 1 : 0;
        mInterrupt->setCallback(this);
        mInterrupt->enable();
    }
}

int Serial::read(char* data, int size)
{
    while (mReadBuffer.used() == 0)
    {
        __asm("wfi");
    }

    return mReadBuffer.read(data, size);
}

unsigned int Serial::write(const char *data, unsigned int size)
{
    unsigned int written = mWriteBuffer.write(data, size);
    triggerWrite();
    return written;
}

bool Serial::pop(char &c)
{
    return mReadBuffer.pop(c);
}

bool Serial::push(char c)
{
    bool success = mWriteBuffer.push(c);
    triggerWrite();
    return success;
}



void Serial::interruptCallback(InterruptController::Index index)
{
    if (mBase->SR.RXNE)
    {
        mReadBuffer.push(mBase->DR);
        mSystem.postEvent(System::Event(mComponent, static_cast<System::Event::Type>(EventType::ReceivedByte)));
    }
    if (mBase->SR.TC)
    {
        char c;
        // check if there is another byte in the buffer and send it or dsiable the interrupt to signal that we are done
        if (mWriteBuffer.pop(c)) mBase->DR = c;
        else mBase->CR1.TCIE = 0;
    }
}

void Serial::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed && mSpeed != 0) setSpeed(mSpeed);
}

void Serial::dmaCallback(Dma::Stream::Callback::Reason reason)
{
//    configDma(nullptr, mDmaRx);
//    mBase->CR3.DMAT = 0;
//    mBase->SR.TC = 1;
//    printf("DMA complete.\n");
//    triggerWrite();
//    return;
    if (reason == Dma::Stream::Callback::Reason::TransferComplete)
    {
        mWriteBuffer.skip(mDmaTransferLength);
        int timeout = 1000;
        while (!mBase->SR.TC && timeout > 0)
        {
            --timeout;
        }
        triggerWrite();
    }
    else
    {
        configDma(nullptr, mDmaRx);
        printf("DMA failed.\n");
    }
}

void Serial::triggerWrite()
{
    if (mDmaTx != 0)
    {
        const char* source;
        mDmaTransferLength = mWriteBuffer.getContBuffer(source);
        if (mDmaTransferLength != 0)
        {
            if (mDmaTransferLength > 10) mDmaTransferLength = 10;
            mDmaTx->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(source));
            mDmaTx->setTransferCount(mDmaTransferLength);
            mBase->SR.TC = 0;
            mDmaTx->start();
        }
    }
    else if (mInterrupt != 0)
    {
        // if we are not transmitting
        if (!mBase->CR1.TCIE)
        {
            mBase->CR1.TCIE = 1;
            char c;
            // send first bye, to start transmission
            if (mWriteBuffer.pop(c)) mBase->DR = c;
        }
    }
    else
    {
        // we have to do it manually
        // TODO: Maybe we could do that with a timer interrupt?
        char c;
        while (mWriteBuffer.pop(c))
        {
            int timeout = 10;
            while (!mBase->SR.TC && timeout > 0)
            {
                mSystem.usleep(1000);
                --timeout;
            }
            mBase->DR = c;
        }
    }
}

