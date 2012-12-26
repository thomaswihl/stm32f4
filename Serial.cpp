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
    Stream<char>(system),
    mBase(reinterpret_cast<volatile USART*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mSpeed(0),
    mDmaTxComplete(*this),
    mDmaRxComplete(*this)
{
    static_assert(sizeof(USART) == 0x1c, "Struct has wrong size, compiler problem.");
    clockControl->addChangeHandler(this);
}

Serial::~Serial()
{
    disable(Device::All);
}

void Serial::setSpeed(uint32_t speed)
{
    uint32_t clock = mClockControl->clock(mClock);
    uint32_t accuracy = 8 * (2 - mBase->CR1.OVER8);
    uint32_t divider = (clock + speed / 2) / speed;
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

void Serial::enable(Device::Part part)
{
    mBase->CR1.UE = 1;
    if (part & Device::Write) mBase->CR1.TE = 1;
    if (part & Device::Read) mBase->CR1.RE = 1;
}

void Serial::disable(Device::Part part)
{
    if (part == Device::All)
    {
        mBase->CR1.UE = 0;
    }
    if (part & Device::Write)
    {
        mBase->CR1.TE = 0;
        mBase->CR1.TCIE = 0;
    }
    if (part & Device::Read)
    {
        mBase->CR1.RE = 0;
        mBase->CR1.RXNEIE = 0;
    }
}

void Serial::config(uint32_t speed, Serial::WordLength dataBits, Serial::Parity parity, Serial::StopBits stopBits, HardwareFlowControl hardwareFlow)
{
    disable(Device::Part::All);
    setSpeed(speed);
    setWordLength(dataBits);
    setParity(parity);
    setStopBits(stopBits);
    setHardwareFlowControl(hardwareFlow);
}

void Serial::configDma(Dma::Stream *tx, Dma::Stream *rx)
{
    mDmaTx = tx;
    mDmaRx = rx;
    if (mDmaTx != nullptr)
    {
        mDmaTx->config(Dma::Stream::Direction::MemoryToPeripheral, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaTx->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaTx->configFifo(Dma::Stream::FifoThreshold::Quater);
        mDmaTx->setEvent(&mDmaTxComplete);
    }
    if (mDmaRx != nullptr)
    {
        mDmaRx->config(Dma::Stream::Direction::PeripheralToMemory, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaRx->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaTx->configFifo(Dma::Stream::FifoThreshold::Quater);
        mDmaRx->setEvent(&mDmaRxComplete);
    }
}

void Serial::configInterrupt(InterruptController::Line* interrupt)
{
    mInterrupt = interrupt;
    if (mInterrupt != nullptr)
    {
        mInterrupt->setCallback(this);
        mInterrupt->enable();
    }
}

void Serial::read(char* data, unsigned int count)
{
    if (readPrepare(data, count))
    {
        simpleRead();
    }
}

void Serial::read(char *data, unsigned int count, System::Event *callback)
{
    if (readPrepare(data, count, callback))
    {
        triggerRead();
    }
}

void Serial::write(const char *data, unsigned int count)
{
    if (writePrepare(data, count))
    {
        simpleWrite();
    }
}

void Serial::write(const char *data, unsigned int count, System::Event *callback)
{
    if (writePrepare(data, count, callback))
    {
        triggerWrite();
    }
}

void Serial::interruptCallback(InterruptController::Index index)
{
    if (mBase->SR.RXNE)
    {
        // check if we need to read another byte, if not dsiable the interrupt
        if (Stream<char>::read(static_cast<char>(mBase->DR))) Stream<char>::readFinished(true);
        else mBase->CR1.RXNEIE = 0;
    }
    if (mBase->SR.TC)
    {
        if (mDmaTx != nullptr)
        {
            mBase->CR1.TCIE = 0;
            mBase->SR.TC = 0;
            Stream<char>::writeFinished(true);
        }
        else
        {
            // check if we need to write another byte, if not dsiable the interrupt
            char c;
            if (Stream<char>::write(c)) mBase->DR = c;
            else mBase->CR1.TCIE = 0;
        }
    }
}

void Serial::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed && mSpeed != 0) setSpeed(mSpeed);
}

void Serial::eventCallback(System::Event *event)
{
    if (event == &mDmaTxComplete)
    {
        mBase->CR3.DMAT = 0;
        if (event->success())
        {
            // If we have an interrupt controller it will signal a TC as soon as the last transfer is complete.
            if (mInterrupt == nullptr)
            {
                // Else we have to wait for it to finish before we can start the next.
                waitTransmitComplete();
                Stream<char>::writeFinished(true);
            }
        }
        else
        {
            // Ooops something went wrong (probably our configuration, anyway, disable DMA.
            configDma(nullptr, mDmaRx);
            Stream<char>::writeFinished(false);
            mSystem.printError("Serial", "DMA write transfer failed");
        }
    }
    else if (event == &mDmaRxComplete)
    {
        mBase->CR3.DMAR = 0;
        if (event->success())
        {
            Stream<char>::readFinished(true);
        }
        else
        {
            // Ooops something went wrong (probably our configuration, anyway, disable DMA.
            configDma(mDmaTx, nullptr);
            Stream<char>::readFinished(false);
            mSystem.printError("Serial", "DMA read transfer failed");
        }
    }
}

void Serial::triggerWrite()
{
    if (mDmaTx != 0 && Stream<char>::mWriteData)
    {
        mBase->CR3.DMAT = 1;
        mDmaTx->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(Stream<char>::mWriteData));
        mDmaTx->setTransferCount(Stream<char>::mWriteCount);
        mBase->SR.TC = 0;
        if (mInterrupt != nullptr) mBase->CR1.TCIE = 1;
        mDmaTx->start();
    }
    else if (mInterrupt != 0)
    {
        mBase->CR1.TCIE = 1;
        // send first bye, to start transmission
        char c;
        if (Stream<char>::write(c)) mBase->DR = c;
        else mBase->CR1.TCIE = 0;
    }
    else
    {
        // we have to do it manually
        simpleWrite();
    }
}

void Serial::triggerRead()
{
    if (mDmaRx != 0)
    {
        mBase->CR3.DMAR = 1;
        mDmaRx->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(Stream<char>::mReadData));
        mDmaRx->setTransferCount(Stream<char>::mReadCount);
        mDmaRx->start();
    }
    else if (mInterrupt != 0)
    {
        mBase->CR1.RXNEIE = 1;
    }
    else
    {
        // we have to do it manually
        simpleRead();
    }
}

void Serial::waitTransmitComplete()
{
    int timeout = 100000;
    while (!mBase->SR.TC && timeout > 0)
    {
        --timeout;
    }
}

void Serial::waitReceiveNotEmpty()
{
    while (!mBase->SR.RXNE)
    {
    }
}

void Serial::simpleRead()
{
    do
    {
        waitReceiveNotEmpty();
    }   while (Stream<char>::read(static_cast<char>(mBase->DR)));
    Stream<char>::readFinished(true);
}

void Serial::simpleWrite()
{
    char c;
    while (Stream<char>::write(c))
    {
        waitTransmitComplete();
        mBase->DR = c;
    }
    Stream<char>::writeFinished(true);
}

