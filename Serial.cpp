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
    mSpeed(0)
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
    mBase->CR1.TE = 1;
    mBase->CR1.RE = 1;
}

void Serial::disable(Device::Part part)
{
    mBase->CR1.UE = 0;
    mBase->CR1.TE = 0;
    mBase->CR1.RE = 0;
}

void Serial::config(uint32_t speed, Serial::WordLength dataBits, Serial::Parity parity, Serial::StopBits stopBits, HardwareFlowControl hardwareFlow)
{
    disable(Device::Part::All);
    setSpeed(speed);
    setWordLength(dataBits);
    setParity(parity);
    setStopBits(stopBits);
    setHardwareFlowControl(hardwareFlow);
    enable(Device::Part::All);
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
        mDmaTx->config(Dma::Stream::Direction::MemoryToPeripheral, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Beats4);
        mDmaTx->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaTx->configFifo(Dma::Stream::FifoThreshold::Quater);
        mBase->CR3.DMAT = 1;
        mDmaTx->setCallback(this);
    }
    mDmaRx = rx;
    if (mDmaRx != nullptr)
    {
        mDmaRx->config(Dma::Stream::Direction::PeripheralToMemory, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Beats4);
        mDmaRx->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaTx->configFifo(Dma::Stream::FifoThreshold::Quater);
        mBase->CR3.DMAR = 1;
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

void Serial::read(char* data, unsigned int count)
{
    readPrepare(data, count);
}

void Serial::read(char *data, unsigned int count, System::Event *callback)
{
    readPrepare(data, count, callback);
}

void Serial::write(const char *data, unsigned int count)
{
    writePrepare(data, count);
    triggerWrite();
    while (mWriteCount != 0) ;
}

void Serial::write(const char *data, unsigned int count, System::Event *callback)
{
    writePrepare(data, count, callback);
    triggerWrite();
}

void Serial::interruptCallback(InterruptController::Index index)
{
    if (mBase->SR.RXNE)
    {
        if (Stream<char>::read(static_cast<char>(mBase->DR))) Stream<char>::readFinished();
        //mSystem.postEvent(System::Event(mComponent, static_cast<System::Event::Type>(EventType::ReceivedByte)));
    }
    if (mBase->SR.TC)
    {
        if (mDmaTx != nullptr)
        {
            mBase->CR1.TCIE = 0;
            mBase->SR.TC = 0;
            Stream<char>::writeFinished();
        }
        else
        {
            char c;
            // check if there is another byte in the buffer and send it or dsiable the interrupt to signal that we are done
            if (Stream<char>::write(c)) mBase->DR = c;
            else mBase->CR1.TCIE = 0;
        }
    }
}

void Serial::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed && mSpeed != 0) setSpeed(mSpeed);
}

void Serial::dmaCallback(Dma::Stream::Callback::Reason reason)
{
    if (reason == Dma::Stream::Callback::Reason::TransferComplete)
    {
        // If we have an interrupt controller it will signal a TC as soon as the last transfer is complete.
        if (mInterrupt == nullptr)
        {
            // Else we have to wait for it to finish before we can start the next.
            waitTransmitComplete();
            Stream<char>::writeFinished();
        }
    }
    else
    {
        // Ooops something went wrong (probably our configuration, anyway, disable DMA.
        configDma(nullptr, mDmaRx);
        mSystem.printError("Serial", "DMA write failed");
    }
}

void Serial::triggerWrite()
{
    if (mDmaTx != 0)
    {
        mDmaTx->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(mWriteData));
        mDmaTx->setTransferCount(mWriteCount);
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
        char c;
        while (Stream<char>::write(c))
        {
            waitTransmitComplete();
            mBase->DR = c;
        }
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

