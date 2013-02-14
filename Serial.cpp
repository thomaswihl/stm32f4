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

Serial::Serial(System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
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
    switch (parity)
    {
    case Parity::None:
        mBase->CR1.PCE = 0;
        mBase->CR1.PS = 0;
        break;
    case Parity::Odd:
        mBase->CR1.PCE = 1;
        mBase->CR1.PS = 1;
        break;
    case Parity::Even:
        mBase->CR1.PCE = 1;
        mBase->CR1.PS = 0;
        break;
    }

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

void Serial::config(uint32_t speed, Serial::Parity parity, Serial::WordLength dataBits, Serial::StopBits stopBits, HardwareFlowControl hardwareFlow)
{
    disable(Device::Part::All);
    setSpeed(speed);
    setParity(parity);
    setWordLength(dataBits);
    setStopBits(stopBits);
    setHardwareFlowControl(hardwareFlow);
}

void Serial::configDma(Dma::Stream *write, Dma::Stream *read)
{
    Device::configDma(write, read);
    if (Device::mDmaWrite != nullptr)
    {
        mDmaWrite->config(Dma::Stream::Direction::MemoryToPeripheral, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaWrite->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaWrite->configFifo(Dma::Stream::FifoThreshold::ThreeQuater);
        mBase->CR3.DMAT = 1;
    }
    else
    {
        mBase->CR3.DMAT = 0;
    }
    if (Device::mDmaRead != nullptr)
    {
        mDmaRead->config(Dma::Stream::Direction::PeripheralToMemory, false, true, Dma::Stream::DataSize::Byte, Dma::Stream::DataSize::Byte, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaRead->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaWrite->configFifo(Dma::Stream::FifoThreshold::Quater);
        mBase->CR3.DMAR = 1;
    }
    else
    {
        mBase->CR3.DMAR = 0;
    }
}

void Serial::interruptCallback(InterruptController::Index index)
{
    uint32_t v = mBase->SR.v;
    __SR* sr = reinterpret_cast<__SR*>(&v);
    if (sr->ORE)
    {
        System::instance()->printError("USART", "Overrun Error");
        Stream<char>::readResult(System::Event::Result::OverrunError);
        System::instance()->debugMsg("O", 1);
    }
    if (sr->FE)
    {
        System::instance()->printError("USART", "Framing Error");
        Stream<char>::readResult(System::Event::Result::FramingError);
        System::instance()->debugMsg("F", 1);
    }
    if (sr->PE)
    {
        System::instance()->printError("USART", "Parity Error");
        Stream<char>::readResult(System::Event::Result::ParityError);
        System::instance()->debugMsg("P", 1);
    }
    if (sr->LBD)
    {
        System::instance()->printError("USART", "Line break");
        Stream<char>::readResult(System::Event::Result::ParityError);
        System::instance()->debugMsg("L", 1);
    }
    if (sr->NF)
    {
        System::instance()->printError("USART", "Noise Error");
        Stream<char>::readResult(System::Event::Result::ParityError);
        System::instance()->debugMsg("N", 1);
    }
    if (sr->RXNE && mBase->CR1.RXNEIE)
    {
        // check if we need to read another byte, if not disable the interrupt
        if (!Stream<char>::read(static_cast<char>(mBase->DR)))
        {
            mBase->CR1.RXNEIE = 0;
        }
    }
    if (sr->TC && mBase->CR1.TCIE)
    {
        // check if we need to write another byte, if not disable the interrupt
        char c;
        if (Stream<char>::write(c))
        {
            mBase->DR = c;
        }
        else
        {
            mBase->CR1.TCIE = 0;
        }
    }
}

void Serial::dmaReadComplete()
{
    Stream<char>::readDmaComplete(mDmaRead->transferCount());
}

void Serial::dmaWriteComplete()
{
    waitTransmitComplete();
    Stream<char>::writeDmaComplete(mDmaWrite->transferCount());
}

void Serial::clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock)
{
    if (reason == ClockControl::Callback::Reason::Changed && mSpeed != 0) setSpeed(mSpeed);
}

void Serial::waitTransmitComplete()
{
    while (!mBase->SR.SR.TC)
    {
    }
}

void Serial::waitReceiveNotEmpty()
{
    while (!mBase->SR.SR.RXNE)
    {
    }
}

void Serial::readPrepare()
{
}

void Serial::readSync()
{
    do
    {
        waitReceiveNotEmpty();
    }   while (Stream<char>::read(static_cast<char>(mBase->DR)));
}

void Serial::readTrigger()
{
    if (mDmaRead != 0)
    {
        if (!mDmaRead->complete()) return;
        char* data;
        unsigned int len;
        readDmaBuffer(data, len);
        if (len > 0)
        {
            mDmaRead->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(data));
            mDmaRead->setTransferCount(len);
            mDmaRead->start();
        }
    }
    else if (mInterrupt != 0)
    {
        mBase->CR1.RXNEIE = 1;
    }
    else
    {
        // we have to do it manually
        readSync();
    }
}

void Serial::readDone()
{
}

void Serial::writePrepare()
{
}

void Serial::writeSync()
{
    char c;
    while (Stream<char>::write(c))
    {
        waitTransmitComplete();
        mBase->DR = c;
    }
}

void Serial::writeTrigger()
{
    if (mDmaWrite != 0)
    {
        if (!mDmaWrite->complete()) return;
        const char* data;
        unsigned int len;
        writeDmaBuffer(data, len);
        if (len > 0)
        {
            mDmaWrite->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(data));
            mDmaWrite->setTransferCount(len);
            mBase->SR.SR.TC = 0;
            mDmaWrite->start();
        }
    }
    else if (mInterrupt != 0)
    {
        if (mBase->CR1.TCIE != 1)
        {
            mBase->CR1.TCIE = 1;
            // send first bye, to start transmission
            char c;
            if (Stream<char>::write(c)) mBase->DR = c;
            else mBase->CR1.TCIE = 0;
        }
    }
    else
    {
        // we have to do it manually
        writeSync();
    }
}

void Serial::writeDone()
{
}

